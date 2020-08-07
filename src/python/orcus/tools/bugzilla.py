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
import pprint
import os
import os.path
import base64
import concurrent.futures as cf


BZURL = "bugs.documentfoundation.org"
CACHE_DIR = os.path.join(os.path.dirname(__file__), ".download-files")
os.makedirs(CACHE_DIR, exist_ok=True)


def get_cache_content(cache_file, func_fetch):
    if os.path.isfile(cache_file):
        with open(cache_file, 'r') as f:
            return f.read()

    s = func_fetch()
    with open(cache_file, 'w') as f:
        f.write(s)

    return s


def get_bug_ids(bz_params):
    """Get all bug ID's for specified bugzilla query parameters."""

    def _fetch():
        r = requests.get(
            f"https://{BZURL}/rest/bug",
            params=bz_params
        )

        if r.status_code != 200:
            raise RuntimeError(f"failed to query bug ids from the TDF bugzilla! (status:{r.status_code})")
        return r.text

    limit = bz_params["limit"]
    offset = bz_params["offset"]
    cache_file = os.path.join(CACHE_DIR, BZURL, f"bug-ids-{limit}-{offset}.json")
    s = get_cache_content(cache_file, _fetch)

    content = json.loads(s)
    bugs = content.get("bugs")
    if not bugs:
        return []

    bug_ids = [bug.get("id") for bug in bugs]
    bug_ids = [x for x in filter(None, bug_ids)]
    return bug_ids


def get_attachments(bug_id):
    """Fetch all attachments for specified bug."""

    def _fetch():
        r = requests.get(f"https://{BZURL}/rest/bug/{bug_id}/attachment")
        if r.status_code != 200:
            raise RuntimeError(f"failed to fetch the attachments for bug {bug_id}! (status:{r.status_code})")
        return r.text

    cache_file = os.path.join(CACHE_DIR, BZURL, f"attachments-{bug_id}.json")
    s = get_cache_content(cache_file, _fetch)
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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--outdir", "-o", type=str, required=True, help="Output directory for downloaded files.")
    parser.add_argument("--limit", type=int, default=500, help="Number of bugs to include in a single search.")
    parser.add_argument("--offset", type=int, default=0, help="Number of bugs to skip in the search results.")
    parser.add_argument("--cont", action="store_true", default=False,
        help="""When specified, the search continues after the initial search
        is returned until the entire search results are exhausted.""")
    parser.add_argument("--worker", type=int, default=8,
        help="Number of worker threads to use for parallel downloads of files.")
    args = parser.parse_args()

    bz_params = {"product": "LibreOffice", "component": "Calc"}
    bz_params["limit"] = args.limit
    bz_params["offset"] = args.offset

    os.makedirs(os.path.join(CACHE_DIR, BZURL), exist_ok=True)

    def _run(bug_id, index, totals):
        """Top-level function for each worker thread."""
        print(f"({index+1}/{totals}) fetching attachments for bug {bug_id} ...", flush=True)

        attachments = get_attachments(bug_id)
        for attachment in attachments:
            filepath = os.path.join(args.outdir, str(bug_id), attachment["filename"])
            os.makedirs(os.path.dirname(filepath), exist_ok=True)
            with open(filepath, "wb") as f:
                f.write(attachment["data"])

    iter_count = 0
    while True:
        bug_ids = get_bug_ids(bz_params)
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
