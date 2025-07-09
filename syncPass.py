#!/usr/bin/env python3
import re
import subprocess
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
            ["gopass", "show", path], capture_output=True, text=True, check=True
        )
        lines = result.stdout.strip().split("\n")
        # remove lines starting with a "[a-z]+: " prefix
        regex = re.compile(r"^\w+: ")
        lines = list(filter(lambda x: not regex.match(x), lines))
        return lines[0]
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to retrieve password for {path}: {e}")
        return None


def export_passwords(mapping, base_url="http://4.3.2.1/editPass"):
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
            # Properly escape the password for URL usage
            escaped_password = urllib.parse.quote(password)
            url = f"{base_url}?id={i}&name={urllib.parse.quote(name)}&layout={layout}&password={escaped_password}"
            logging.info(f"Exporting password for {name}")
            try:
                # print(url)
                response = requests.get(url)
                logging.info(f"Export status: {response.status_code}")
            except Exception as e:
                logging.error(f"Error exporting {name}: {e}")


# Example usage
if __name__ == "__main__":
    passphrase = input("Passphrase: ")

    url = f"http://4.3.2.1/passphrase?p={urllib.parse.quote(passphrase)}"
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
        export_passwords(exported)
