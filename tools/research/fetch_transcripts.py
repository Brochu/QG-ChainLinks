# Fetches YouTube transcripts for every link in links.txt and stores them
# as plain text in transcripts/<video_id>.txt. Already-fetched videos are
# skipped, so re-running after adding new links only fetches the new ones.
#
# Usage: python fetch_transcripts.py

import re
import sys
from pathlib import Path

import requests
from youtube_transcript_api import YouTubeTranscriptApi

HERE = Path(__file__).parent
LINKS_FILE = HERE / "links.txt"
OUT_DIR = HERE / "transcripts"


def extract_video_id(url):
    for pattern in (r"v=([\w-]{11})", r"youtu\.be/([\w-]{11})", r"shorts/([\w-]{11})"):
        m = re.search(pattern, url)
        if m:
            return m.group(1)
    return None


def fetch_title(video_id):
    try:
        r = requests.get(
            "https://www.youtube.com/oembed",
            params={"url": f"https://www.youtube.com/watch?v={video_id}", "format": "json"},
            timeout=10,
        )
        return r.json()["title"]
    except Exception:
        return "(title unavailable)"


def main():
    if not LINKS_FILE.exists():
        sys.exit(f"No links file found at {LINKS_FILE}")

    OUT_DIR.mkdir(exist_ok=True)
    api = YouTubeTranscriptApi()

    lines = LINKS_FILE.read_text(encoding="utf-8").splitlines()
    urls = [l.strip() for l in lines if l.strip() and not l.strip().startswith("#")]

    for url in urls:
        video_id = extract_video_id(url)
        if not video_id:
            print(f"SKIP (couldn't find a video id): {url}")
            continue

        out_file = OUT_DIR / f"{video_id}.txt"
        if out_file.exists():
            print(f"skip (already fetched): {video_id}")
            continue

        try:
            transcript = api.fetch(video_id, languages=["en"])
        except Exception as e:
            print(f"FAILED {video_id}: {type(e).__name__}: {e}")
            continue

        title = fetch_title(video_id)
        text = "\n".join(snippet.text for snippet in transcript)
        out_file.write_text(f"{title}\n{url}\n\n{text}\n", encoding="utf-8")
        print(f"fetched: {video_id} - {title}")


if __name__ == "__main__":
    main()
