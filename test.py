#!/bin/env python
# HTTP Service
# run using:
# uvicorn thismodule:app --host 0.0.0.0 --reload --port 5000 --log-level=debug
import os
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import ORJSONResponse, HTMLResponse, PlainTextResponse
import random

debug = bool(os.environ.get("DEBUG", False))

passwords: list[dict[str, str | int]] = []

for n in range(12):
    passwords.append(
        {
            "name": f"example{n}",
            "layout": int(random.random()),
            "password": "irrelevant",
            "len": 12,
        }
    )


if debug:
    app = FastAPI(debug=True, default_response_class=ORJSONResponse)
    app.add_middleware(
        CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"]
    )
else:
    app = FastAPI(
        debug=False,
        docs_url=None,
        redoc_url=None,
        default_response_class=ORJSONResponse,
    )


# return the local index.html
@app.get("/", response_class=HTMLResponse)
async def index():
    "Returns the local index.html"
    with open("index.html", "r") as f:
        content = f.read()
    return HTMLResponse(content=content)


@app.get("/passphrase")
async def passphrase(p: str):
    print("set pass phrase")


@app.get("/typePass")
async def typePass(id: int):
    print(passwords[id])


@app.get("/typeRaw")
async def typeRaw(text: str, layout: int):
    print(text)


@app.get("/fetchPass", response_class=PlainTextResponse)
async def fetchPass(id: int):
    return passwords[id]["password"] if id < len(passwords) else None


@app.get("/editPass")
async def editPass(
    id: int,
    layout: int | None = None,
    name: str | None = None,
    password: str | None = None,
):
    obj = {
        k: v
        for k, v in {
            "uid": id,
            "layout": layout,
            "name": name,
            "password": password,
        }.items()
        if v is not None
    }
    if id < len(passwords):
        passwords[id].update(obj)
    else:
        obj["uid"] = len(passwords)
        passwords.append(obj)


@app.get("/list")
async def listPasswords():
    for i, p in enumerate(passwords):
        p["uid"] = i

    return {"passwords": passwords, "free": 9999}


if __name__ == "__main__":
    import uvicorn
    import logging
    from logging import StreamHandler, DEBUG

    uvicorn_log = logging.getLogger("uvicorn")
    uvicorn_log.setLevel(DEBUG)
    uvicorn_log.addHandler(StreamHandler())
    # run the app
    uvicorn.run(app, host="0.0.0.0")
