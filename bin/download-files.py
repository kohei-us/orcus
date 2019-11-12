#!/usr/bin/env python3

import argparse
import requests
import json
import pprint
import os
import os.path
import base64


BZURL = "bugs.documentfoundation.org"
BZ_PARAMS = {"product": "LibreOffice", "component": "Calc"}
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


def get_bug_ids():

    def _fetch():
        r = requests.get(
            f"https://{BZURL}/rest/bug",
            params=BZ_PARAMS
        )

        if r.status_code != 200:
            raise RuntimeError("failed to query bug ids from the TDF bugzilla!")
        return r.text

    cache_file = os.path.join(CACHE_DIR, BZURL, "bug-ids.json")
    s = get_cache_content(cache_file, _fetch)

    content = json.loads(s)
    bugs = content.get("bugs")
    if not bugs:
        return []

    bug_ids = [bug.get("id") for bug in bugs]
    bug_ids = [x for x in filter(None, bug_ids)]
    return bug_ids


def get_attachments(bug_id):

    def _fetch():
        r = requests.get(f"https://{BZURL}/rest/bug/{bug_id}/attachment")
        if r.status_code != 200:
            raise RuntimeError(f"failed to fetch the attachments for bug {bug_id}!")
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
    parser.add_argument("--outdir", "-o", type=str, required=True)
    parser.add_argument("--limit", type=int, default=10)
    args = parser.parse_args()
    BZ_PARAMS["limit"] = args.limit

    os.makedirs(os.path.join(CACHE_DIR, BZURL), exist_ok=True)
    for bug_id in get_bug_ids():
        print(f"fetching attachments for bug {bug_id} ...", flush=True)
        attachments = get_attachments(bug_id)
        for attachment in attachments:
            filepath = os.path.join(args.outdir, str(bug_id), attachment["filename"])
            os.makedirs(os.path.dirname(filepath), exist_ok=True)
            with open(filepath, "wb") as f:
                f.write(attachment["data"])


if __name__ == "__main__":
    main()
