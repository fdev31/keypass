#!/usr/bin/env python3
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
        return lines[-1]
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to retrieve password for {path}: {e}")
        return None


def export_passwords(mapping, base_url="http://foobar.com/editPass"):
    """Export passwords to the specified URL"""
    for i, (name, path) in enumerate(mapping.items()):
        if path.startswith("gopass:"):
            password = get_password_from_gopass(path.split(":", 1)[1])
        else:
            password = path
        if password:
            # Properly escape the password for URL usage
            escaped_password = urllib.parse.quote(password)
            url = f"{base_url}?id={i}&name={urllib.parse.quote(name)}&password={escaped_password}"
            print(url)

            logging.info(f"Exporting password for {name}")
            try:
                response = requests.get(url)
                logging.info(f"Export status: {response.status_code}")
            except Exception as e:
                logging.error(f"Error exporting {name}: {e}")


# Example usage
if __name__ == "__main__":
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
