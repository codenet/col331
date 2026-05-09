import os
import requests
import json

def send_notification(web_hook, pay_load):
    resp = requests.post(web_hook, json=pay_load)
    if resp.status_code != 204:
        print(f"Failed to send notification.\n Status code: {resp.status_code} \n Response: {resp.text}")
        exit(1)


def get_discord_payload_v0(title, url, author, branch):
    payload = {
        "embeds": [
            {
                "title": "PR Merged",
                "url": url,
                "color": 3066993,  # green in decimal (0x2ecc71 in hex)
                "fields": [
                    {
                        "name": "Title",
                        "value": title,
                        "inline": False
                    },
                    {
                        "name": "Author",
                        "value": author,
                        "inline": True
                    },
                    {
                        "name": "Target Branch",
                        "value": branch,
                        "inline": True
                    }
                ],
                "footer": {
                    "text": f"Branch: {branch}"
                }
            }
        ]
    }
    
    return payload
    
def get_discord_payload_v1(title, url, author, branch, number):
    payload = {
        "embeds": [
            {
                "title": f"PR #{number}: {title}",
                "url": url,
                "color": 3066993,  
                "description": f"Merged into **{branch}**",
                "fields": [
                    {
                        "name": "Author",
                        "value": author,
                        "inline": True
                    },
                    {
                        "name": "Branch",
                        "value": branch,
                        "inline": True
                    }
                ],
                "footer": {
                    "text": "GitHub PR Notification"
                }
            }
        ]
    }
    return payload

event = os.environ.get("EVENT_NAME")
web_hook = os.environ["WEBHOOK"] # Discord Webhook URL presented as an environment variable
NEW_PART = "newpart"

if event == "pull_request_target":
    merged = (os.environ.get("PR_MERGED") == "true")
    labels = [label["name"] for label in json.loads(os.environ.get("PR_LABELS", "[]"))]
    # Only act notify if the pull request is merged
    if not merged:
        print("Pull request is not merged. No notification will be sent.")
        exit(0)
        
    if NEW_PART not in labels:
        print(f"Pull request does not have the '{NEW_PART}' label. No notification will be sent.")
        exit(0)
    
    pr_title  = os.environ.get("PR_TITLE")
    pr_url    = os.environ.get("PR_URL")
    pr_author = os.environ.get("PR_AUTHOR")
    pr_target_branch = os.environ.get("TARGET_BRANCH")
    pr_number = os.environ.get("PR_NUMBER")
    
    payload = get_discord_payload_v1(pr_title, pr_url, pr_author,
                                     pr_target_branch,pr_number)
    
    send_notification(web_hook, payload)