#!/usr/bin/env python3
import sys
import re
import subprocess
from getpass import getpass
import shlex
import urllib.parse
import json
import requests
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s"
)


def get_password_from_gopass(path):
    """Retrieve password from gopass using the provided path"""
    try:
        result = subprocess.run(
            ["gopass", "show", "-o", path], capture_output=True, text=True, check=True
        )
        lines = result.stdout.strip().split("\n")
        # remove lines starting with a "[a-z]+: " prefix
        regex = re.compile(r"^\w+: ")
        lines = list(filter(lambda x: not regex.match(x), lines))
        return lines[-1]
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to retrieve password for {path}: {e}")
        return None


def export_passwords(mapping, callback):
    """Export passwords to the specified URL"""
    for i, (name, data) in enumerate(mapping.items()):
        if isinstance(data, str):
            path = data
            layout = 0
        else:
            (path, layout) = data
        if path.startswith("gopass:"):
            password = get_password_from_gopass(path.split(":", 1)[1])
        else:
            password = path
        if password:
            callback(i, name, layout, password)


def sync2key(index, name, layout, password):
    base_url = "http://4.3.2.1/editPass"
    escaped_password = urllib.parse.quote(password)
    url = f"{base_url}?id={index}&name={urllib.parse.quote(name)}&layout={layout}&password={escaped_password}"
    logging.info(f"Exporting password for {name}")
    try:
        response = requests.get(url)
        logging.info(f"Export status: {response.status_code}")
    except Exception as e:
        logging.error(f"Error exporting {name}: {e}")


def dumpBackupline(index, name, layout, password):
    # "Syntax: <passphrase> <slot> <layout> <name> <pass>")
    print(
        subprocess.getstatusoutput(
            f"./standalone/encoderaw {passphrase} {index} {layout} {shlex.quote(name)} {shlex.quote(password)}"
        )[1]
    )


# Example usage
if __name__ == "__main__":
    sys.stderr.write("Enter passphrase: ")
    sys.stderr.flush()
    passphrase = getpass("").strip()

    # read exported data from a JSON file
    try:
        exported = json.load(open("exported.json", "r"))
    except:
        print(""" Can't open the exported.json file, ensure you have one with such format:
{
    "pin": "12345",
    "google": "gopass:websites/google.com/john@doe.com",
}
        """)
    else:
        dump = "--dump" in sys.argv
        debug = "--debug" in sys.argv
        if debug:
            dump = True
        if not dump:
            url = f"http://4.3.2.1/passphrase?p={urllib.parse.quote(passphrase)}"
            requests.get(url)
        else:
            print("\n#KPDUMP")
        if debug:
            export_passwords(exported, print)
        else:
            export_passwords(exported, dumpBackupline if dump else sync2key)
        if dump:
            print("#/KPDUMP")
