#!/bin/env python
# HTTP Service
# run using:
# uvicorn thismodule:app --host 0.0.0.0 --reload --port 5000 --log-level=debug
import os
import asyncio
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import ORJSONResponse, HTMLResponse

debug = bool(os.environ.get("DEBUG", False))

passwords: list[dict[str, str | int]] = [
    {"name": "example", "layout": 1, "password": "abc"},
    {"name": "example2", "layout": 0, "password": "123"},
]


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


@app.get("/typePass")
async def typePass(id: int):
    print(passwords[id])


@app.get("/typeRaw")
async def typeRaw(text: str, layout: int):
    print(text)


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
        passwords[id].update()
    else:
        obj["uid"] = len(passwords)
        passwords.append(obj)


@app.get("/list")
async def listPasswords():
    for i, p in enumerate(passwords):
        p["uid"] = i

    return {"passwords": passwords}


if __name__ == "__main__":
    import uvicorn
    import logging
    from logging.config import dictConfig
    from logging import StreamHandler, DEBUG
    from logging.handlers import RotatingFileHandler
    from pathlib import Path

    uvicorn_log = logging.getLogger("uvicorn")
    uvicorn_log.setLevel(DEBUG)
    uvicorn_log.addHandler(StreamHandler())
    # run the app
    uvicorn.run(app, host="localhost")
