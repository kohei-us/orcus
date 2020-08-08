########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import argparse
import requests
import json
import os
import base64
import concurrent.futures as cf
from pathlib import Path
from urllib.parse import urlparse


class BugzillaAccess:

    def __init__(self, bzurl, cache_dir):
        self._bzurl = bzurl
        self._cache_dir = cache_dir
        os.makedirs(self._cache_dir, exist_ok=True)

    def _get_cache_content(self, cache_file, func_fetch):
        if os.path.isfile(cache_file):
            with open(cache_file, 'r') as f:
                return f.read()

        s = func_fetch()
        with open(cache_file, 'w') as f:
            f.write(s)

        return s

    def get_bug_ids(self, bz_params):
        """Get all bug ID's for specified bugzilla query parameters."""

        def _fetch():
            r = requests.get(
                f"{self._bzurl}/rest/bug",
                params=bz_params
            )

            if r.status_code != 200:
                raise RuntimeError(f"failed to query bug ids from the TDF bugzilla! (status:{r.status_code})")
            return r.text

        escape_chars = " /"
        buf = []
        for key in bz_params.keys():
            v = str(bz_params[key])
            for c in escape_chars:
                v = v.replace(c, '-')
            buf.append(key)
            buf.append(v)

        cache_file = '-'.join(buf) + ".json"
        cache_file = self._cache_dir / cache_file
        s = self._get_cache_content(cache_file, _fetch)

        content = json.loads(s)
        bugs = content.get("bugs")
        if not bugs:
            return []

        bug_ids = [bug.get("id") for bug in bugs]
        bug_ids = [x for x in filter(None, bug_ids)]
        return bug_ids

    def get_attachments(self, bug_id):
        """Fetch all attachments for specified bug."""

        def _fetch():
            r = requests.get(f"{self._bzurl}/rest/bug/{bug_id}/attachment")
            if r.status_code != 200:
                raise RuntimeError(
                    f"failed to fetch the attachments for bug {bug_id}! (status:{r.status_code})")
            return r.text

        cache_file = self._cache_dir / f"attachments-{bug_id}.json"
        s = self._get_cache_content(cache_file, _fetch)
        content = json.loads(s)
        attachments = list()
        for d in content["bugs"][str(bug_id)]:
            data = d["data"]
            bytes = base64.b64decode(data)
            attachments.append({
                "content_type": d["content_type"],
                "filename": d["file_name"],
                "data": bytes
            })
        return attachments


def parse_query_params(queries):
    bz_params = dict()
    for query in queries:
        k, v = query.split('=')
        if v and v[0] in ('"', "'"):
            if v[0] != v[-1]:
                raise argparse.ArgumentError(f"mis-matched quotes in {query}")
            v = v[1:-1]
        bz_params[k] = v
    return bz_params


def main():
    parser = argparse.ArgumentParser(description="Tool to download attachments from a bugzilla database.")
    parser.add_argument("--outdir", "-o", type=str, required=True, help="Output directory for downloaded files.")
    parser.add_argument("--limit", type=int, default=50, help="Number of bugs to include in a single search.")
    parser.add_argument("--offset", type=int, default=0, help="Number of bugs to skip in the search results.")
    parser.add_argument(
        "--cont", action="store_true", default=False,
        help="""When specified, the search continues after the initial search
        is returned until the entire search results are exhausted.""")
    parser.add_argument(
        "--worker", type=int, default=8,
        help="Number of worker threads to use for parallel downloads of files.")
    parser.add_argument(
        "--cache-dir", type=Path, default=Path(".bugzilla"),
        help="Directory to keep cached downloads.")
    parser.add_argument(
        "--url", type=str, required=True, help="Base URL for bugzilla service.")
    parser.add_argument("query", type=str, nargs='*')
    args = parser.parse_args()

    bz_params = parse_query_params(args.query)

    for k, v in bz_params.items():
        print(f"{k}: {v}")

    bz_params["limit"] = args.limit
    bz_params["offset"] = args.offset

    url = urlparse(args.url)
    cache_dir = Path(args.cache_dir) / url.netloc
    bz = BugzillaAccess(args.url, cache_dir)

    def _run(bug_id, index, totals):
        """Top-level function for each worker thread."""
        width = len(str(totals))
        index_s = str(index+1)
        index_s = ' ' * (width - len(index_s)) + index_s
        print(f"({index_s}/{totals}) fetching attachments for bug {bug_id} ...", flush=True)

        try:
            attachments = bz.get_attachments(bug_id)
            for attachment in attachments:
                filepath = Path(args.outdir) / url.netloc / str(bug_id) / attachment["filename"]
                os.makedirs(os.path.dirname(filepath), exist_ok=True)
                with open(filepath, "wb") as f:
                    f.write(attachment["data"])
        except Exception as e:
            print(e)

    iter_count = 0
    while True:
        bug_ids = bz.get_bug_ids(bz_params)
        if not bug_ids:
            return
        print(f"-- iteration {iter_count+1}", flush=True)
        with cf.ThreadPoolExecutor(max_workers=args.worker) as executor:
            for i, bug_id in enumerate(bug_ids):
                executor.submit(_run, bug_id, i, len(bug_ids))

        if not args.cont:
            return

        bz_params["offset"] += bz_params["limit"]
        iter_count += 1


if __name__ == "__main__":
    main()
