#!/usr/bin/env python3
"""Prints a Paged.js page to PDF with headless Chrome, through the
DevTools protocol (as pagedjs-cli does): opens the page, waits until
Paged.js has laid out every page (the page sets window.__pagedDone in
PagedConfig.after), then prints at the CSS page size.

Usage: print-pdf.py <chrome> <page.html> <out.pdf>"""

import base64, json, os, subprocess, sys, tempfile, time, urllib.request
import websocket

chrome, page, out = sys.argv[1], os.path.abspath(sys.argv[2]), sys.argv[3]
port = 9339
profile = tempfile.mkdtemp(prefix="mm-chrome-")
proc = subprocess.Popen([chrome, "--headless", "--disable-gpu",
                         "--allow-file-access-from-files",
                         "--remote-debugging-port=%d" % port,
                         "--user-data-dir=" + profile, "about:blank"],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    for _ in range(100):
        try:
            targets = json.load(urllib.request.urlopen("http://127.0.0.1:%d/json/list" % port))
            target = [t for t in targets if t["type"] == "page"][0]
            break
        except Exception:
            time.sleep(0.1)
    else:
        sys.exit("Chrome didn't start")
    ws = websocket.create_connection(target["webSocketDebuggerUrl"], timeout=120,
                                  suppress_origin=True)
    ids = iter(range(1, 1000000))

    def call(method, **params):
        n = next(ids)
        ws.send(json.dumps({"id": n, "method": method, "params": params}))
        while True:
            msg = json.loads(ws.recv())
            if msg.get("id") == n:
                if "error" in msg:
                    sys.exit("%s: %s" % (method, msg["error"]))
                return msg.get("result", {})
            if msg.get("method") == "Runtime.consoleAPICalled":
                args = " ".join(str(a.get("value", a.get("description", ""))) for a in msg["params"]["args"])
                print("console:", args)
            if msg.get("method") == "Runtime.exceptionThrown":
                print("exception:", msg["params"]["exceptionDetails"].get("exception", {}).get("description"))

    call("Runtime.enable")
    call("Page.enable")
    call("Page.navigate", url="file://" + page)
    deadline = time.time() + 120
    while time.time() < deadline:
        done = call("Runtime.evaluate", expression="window.__pagedDone === true")
        if done.get("result", {}).get("value"):
            break
        time.sleep(0.3)
    else:
        sys.exit("Paged.js didn't finish within 2 minutes")
    pages = call("Runtime.evaluate",
                 expression="document.querySelectorAll('.pagedjs_page').length")["result"]["value"]
    pdf = call("Page.printToPDF", preferCSSPageSize=True, printBackground=True,
               displayHeaderFooter=False)
    open(out, "wb").write(base64.b64decode(pdf["data"]))
    print("wrote %s: %d pages" % (out, pages))
finally:
    proc.terminate()
    proc.wait()
