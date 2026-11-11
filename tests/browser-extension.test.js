"use strict";

const assert = require("node:assert/strict");
const crypto = require("node:crypto");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");

const source = fs.readFileSync(path.join(__dirname, "../browser-extension/background.js"), "utf8");

function createHarness(tabs, userAgent = "Chrome/120", brave = undefined) {
    const messages = [];
    const scriptCalls = [];
    const listener = { addListener() {} };
    let onNativeMessage = null;
    tabs.forEach((tab, index) => {
        if(tab.id === undefined)
            tab.id = index + 1;
        if(tab.audible === undefined)
            tab.audible = true;
    });
    const context = {
        navigator: { userAgent, brave },
        console,
        AbortSignal,
        btoa: value => Buffer.from(value, "binary").toString("base64"),
        fetch: async () => ({ ok: true, body: new Blob(["icon"]).stream() }),
        chrome: {
            tabs: {
                query: async query => query.audible ? tabs.filter(tab => tab.audible) : tabs.filter(tab => tab.active),
                get: async id => {
                    const tab = tabs.find(candidate => candidate.id === id);
                    if(!tab)
                        throw new Error("Tab closed");
                    return tab;
                },
                onUpdated: listener, onRemoved: listener, onCreated: listener, onActivated: listener
            },
            scripting: {
                executeScript: async details => {
                    scriptCalls.push(details);
                }
            },
            alarms: { onAlarm: listener, create() {} },
            runtime: {
                connectNative: () => ({
                    onDisconnect: listener,
                    onMessage: { addListener: callback => { onNativeMessage = callback; } },
                    postMessage: value => messages.push(value)
                })
            }
        }
    };
    vm.createContext(context);
    vm.runInContext(source, context);
    return {
        context,
        messages,
        scriptCalls,
        sendNativeMessage: message => onNativeMessage && onNativeMessage(message)
    };
}

test("sends the one audible tab and its icon", async () => {
    const harness = createHarness([{ title: "Song", favIconUrl: "https://example.com/icon.png" }]);
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).title, "Song");
    assert.equal(harness.messages.at(-1).icon, "aWNvbg==");
    assert.equal(harness.messages.at(-1).browser, "chrome");
});

test("does not choose between multiple audible tabs", async () => {
    const harness = createHarness([{ title: "One" }, { title: "Two" }], "Firefox/130");
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).audibleCount, 2);
    assert.equal(harness.messages.at(-1).icon, "");
    assert.equal(harness.messages.at(-1).browser, "firefox");
});

test("does not fetch unsupported icon URLs", async () => {
    const harness = createHarness([{ title: "Song", favIconUrl: "file:///private" }], "Chrome/120 Edg/120");
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).icon, "");
    assert.equal(harness.messages.at(-1).browser, "msedge");
});

test("limits downloaded icon data", async () => {
    const harness = createHarness([{ title: "Song", favIconUrl: "https://example.com/icon.png" }]);
    harness.context.fetch = async () => ({ ok: true, body: new Blob([Buffer.alloc(90 * 1024)]).stream() });
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).icon, "");
});

test("development manifest key matches the registered extension ID", () => {
    const manifest = JSON.parse(fs.readFileSync(path.join(__dirname, "../browser-extension/manifest.chromium.json")));
    const digest = crypto.createHash("sha256").update(Buffer.from(manifest.key, "base64")).digest().subarray(0, 16);
    const id = [...digest].flatMap(byte => [byte >> 4, byte & 15]).map(value => String.fromCharCode(97 + value)).join("");
    assert.equal(id, "pmblljpkoajpojjjeidepiddahgpilge");
});

test("both browser manifests grant the scripting permission", () => {
    for(const browser of ["chromium", "firefox"]) {
        const manifest = JSON.parse(fs.readFileSync(path.join(__dirname, `../browser-extension/manifest.${browser}.json`)));
        assert.ok(manifest.permissions.includes("scripting"));
        assert.equal(manifest.version, "1.1.0");
    }
});

test("identifies Brave independently from Chrome", async () => {
    const harness = createHarness([{ title: "Song" }], "Chrome/120", { isBrave: async () => true });
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).browser, "brave");
});

test("keeps the last audible tab after playback pauses", async () => {
    const tabs = [{ title: "Song", favIconUrl: "https://example.com/icon.png" }];
    const harness = createHarness(tabs);
    await vm.runInContext("updateArtwork()", harness.context);
    tabs[0].audible = false;
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).audibleCount, 0);
    assert.equal(harness.messages.at(-1).title, "Song");
    assert.equal(harness.messages.at(-1).icon, "aWNvbg==");
    tabs.pop();
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).icon, "");
});

test("uses the active tab when no audible tab has been identified", async () => {
    const harness = createHarness([{ title: "Video", active: true, audible: false,
                                     favIconUrl: "https://example.com/icon.png" }]);
    await vm.runInContext("updateArtwork()", harness.context);
    assert.equal(harness.messages.at(-1).audibleCount, 0);
    assert.equal(harness.messages.at(-1).title, "Video");
    assert.equal(harness.messages.at(-1).icon, "aWNvbg==");
});

test("pauses media in all audible tabs concurrently", async () => {
    const harness = createHarness([
        { title: "First", audible: true },
        { title: "Silent", audible: false },
        { title: "Second", audible: true }
    ]);

    await vm.runInContext("updateArtwork()", harness.context);
    harness.sendNativeMessage({ command: "pauseAudibleTabs" });
    await new Promise(resolve => setImmediate(resolve));

    assert.deepEqual(harness.scriptCalls.map(call => call.target.tabId), [1, 3]);
    assert.ok(harness.scriptCalls.every(call => call.target.allFrames));
    assert.ok(harness.scriptCalls.every(call => call.func.toString().includes("media.pause()")));
});
