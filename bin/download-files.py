#!/usr/bin/env python3

import argparse
import requests
import json
import pprint


def main():
    parser = argparse.ArgumentParser()
    args = parser.parse_args()

    r = requests.get(
        "https://bugs.documentfoundation.org/rest/bug",
        params={"product": "LibreOffice", "component": "Calc", "limit": 10}
    )

    if r.status_code != 200:
        return

    content = json.loads(r.text)
    bugs = content.get("bugs")
    if not bugs:
        return

    bug_ids = [bug.get("id") for bug in bugs]
    bug_ids = [x for x in filter(None, bug_ids)]
    print(bug_ids)


if __name__ == "__main__":
    main()
