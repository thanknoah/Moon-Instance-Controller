import requests
import os
import sys
import urllib.parse
import atexit
import time

VERSION = "V4"

def delete_self():
    script_path = os.path.abspath(sys.argv[0])
    print(f"Scheduling self-delete: {script_path}")
    atexit.register(lambda: os.remove(script_path))

def delete_files():
    files_to_delete = [
        "Moon Instance Controller.exe",
        "Moon Installer.exe"
    ]   

    for file in files_to_delete:
        try:
            os.remove(file)
            print(f"Deleted: {file}")
        except FileNotFoundError:
            print(f"File not found: {file}")
        except PermissionError:
            print(f"Permission denied to delete: {file}")
        except Exception as e:
            print(f"Error deleting {file}: {e}")

def download_github_file(url, filename=None):
    if filename is None:
        filename = urllib.parse.unquote(os.path.basename(url))  # decode %20 to space

    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()

        with open(filename, "wb") as f:
            f.write(response.content)

        print(f"✅ Downloaded: {filename}")

    except requests.exceptions.HTTPError as http_err:
        print(f"❌ HTTP error: {http_err}")
        sys.exit(1)
    except requests.exceptions.ConnectionError:
        print("❌ Connection error. Check your internet.")
        sys.exit(1)  
    except requests.exceptions.Timeout:
        print("❌ Timeout error.")
        sys.exit(1) 
    except Exception as err:
        print(f"❌ Unexpected error: {err}")
        sys.exit(1) 

def checkingForNewUpdates():
    url = "https://raw.githubusercontent.com/thanknoah/Moon-Instance-Controller/main/version.txt"

    try:
        response = requests.get(url)
        response.raise_for_status()

        if VERSION in response.text:
            print("No new updates detected.. Closing..")
            time.sleep(2)
            sys.exit()
    except Exception as e:
        print(f"Failed to fetch version file: {e}")
        sys.exit(1)

# First: Check for updates
checkingForNewUpdates()

# Then: Delete old files
delete_files()

# Then: Download new files
download_github_file("https://raw.githubusercontent.com/thanknoah/Moon-Instance-Controller/main/Moon%20Installer.exe")
download_github_file("https://raw.githubusercontent.com/thanknoah/Moon-Instance-Controller/main/Moon%20Instance%20Controller.exe")

time.sleep(2)

# Finally: Schedule self-delete
delete_self()
