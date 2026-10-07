#!/usr/bin/env node
/* Runner for the FastRPC conformance vectors (email/fastrpc-conformance)
 * for the JavaScript and TypeScript modules.
 *
 *   node test/conformance-runner.mjs [javascript/fastrpc.mjs | typescript/fastrpc.ts]
 *
 * Reads JSON requests from stdin and answers one JSON line per request, see
 * FORMAT.md (Runner protocol) in fastrpc-conformance. The default module is
 * javascript/fastrpc.mjs (libfastrpc-js); with typescript/fastrpc.ts it is
 * libfastrpc-ts (node >= 22.18 strips the types).
 *
 * What the modules cannot express is reported as they behave:
 *  - a decoded number does not say whether it was an Integer or a Double;
 *    integral numbers are reported as int, the others as double,
 *  - a decoded DateTime is a Date in the local time of the machine,
 *  - faults are thrown as Error("FRPC/<code>: <message>"),
 *  - there is no API to encode a method response or a fault: a response is
 *    the header and the response tag around serialize(), as serializeCall()
 *    does for calls; a fault cannot be encoded.
 */

import * as fs from "fs";
import * as path from "path";
import * as readline from "readline";
import { fileURLToPath } from "url";

const here = path.dirname(fileURLToPath(import.meta.url));
const modulePath = path.resolve(process.argv[2]
	|| path.join(here, "../javascript/fastrpc.mjs"));
const frpc = await import(modulePath);
const NAME = modulePath.endsWith(".ts") ? "libfastrpc-ts" : "libfastrpc-js";
const VERSION = JSON.parse(fs.readFileSync(
	path.join(here, "../typescript/package.json"), "utf8")).version;

const TYPE_CALL = 13;
const QUARTER = 15;

class Unrepresentable extends Error {}
class UnsupportedRevision extends Error {}

const te = new TextEncoder();
const td = new TextDecoder("utf-8", { fatal: true });

function toHex(bytes) {
	return Array.from(bytes, b => b.toString(16).padStart(2, "0"))
		.join("").toUpperCase();
}

function fromHex(hex) {
	let bytes = new Uint8Array(hex.length / 2);
	for (let i = 0; i < bytes.length; i++) {
		bytes[i] = parseInt(hex.substr(2 * i, 2), 16);
	}
	return bytes;
}

// decoding: module values -> canonical JSON

function doubleBits(value) {
	let view = new DataView(new ArrayBuffer(8));
	view.setFloat64(0, value);
	return toHex(new Uint8Array(view.buffer));
}

function canonical(value) {
	if (value === null) { return { null: null }; }
	if (value instanceof ArrayBuffer) {
		return { binary: toHex(new Uint8Array(value)) };
	}
	if (value instanceof Date) {
		if (isNaN(value.getTime())) {
			throw new Unrepresentable("Invalid Date");
		}
		return { datetime: {
			ts: String(value.getTime() / 1000),
			zone: value.getTimezoneOffset() / QUARTER,
			fields: [value.getFullYear(), value.getMonth() + 1,
			         value.getDate(), value.getHours(), value.getMinutes(),
			         value.getSeconds(), value.getDay()],
		} };
	}
	if (Array.isArray(value)) { return { array: value.map(canonical) }; }
	switch (typeof value) {
	case "boolean": return { bool: value };
	case "string": return { string: toHex(te.encode(value)) };
	case "number":
		if (Number.isInteger(value) && !Object.is(value, -0)) {
			return { int: BigInt(value).toString() };
		}
		return { double_bits: doubleBits(value) };
	case "object":
		return { struct: Object.entries(value).map(
			([name, member]) => [toHex(te.encode(name)), canonical(member)]) };
	}
	throw new Error(`unexpected decoded value ${typeof value}`);
}

function decode(bytes) {
	let result;
	try {
		result = frpc.parse(bytes, { arrayBuffers: true });
	} catch (e) {
		let fault = /^FRPC\/(-?\d+): ([\s\S]*)$/.exec(e.message);
		if (!fault) { throw e; }
		return { fault: { code: fault[1],
		                  message: toHex(te.encode(fault[2])) } };
	}
	if (bytes.length > 4 && (bytes[4] >> 3) == TYPE_CALL) {
		return { call: { name: toHex(te.encode(result.method)),
		                 params: result.params.map(canonical) } };
	}
	return { response: canonical(result) };
}

// encoding: canonical JSON -> module values -> octets

function text(hex) {
	try {
		return td.decode(fromHex(hex));
	} catch (e) {
		throw new Unrepresentable("a JS string cannot hold invalid UTF-8");
	}
}

function native(value) {
	let [[tag, item]] = Object.entries(value);
	switch (tag) {
	case "null": return null;
	case "bool": return item;
	case "int": {
		let n = BigInt(item);
		if (n > BigInt(Number.MAX_SAFE_INTEGER)
				|| n < BigInt(Number.MIN_SAFE_INTEGER)) {
			throw new Unrepresentable(`${item} is not a safe JS integer`);
		}
		return Number(n);
	}
	case "double": return { _hint: "float", value: Number(item) };
	case "double_bits": {
		let view = new DataView(fromHex(item).buffer);
		return { _hint: "float", value: view.getFloat64(0) };
	}
	case "string": return text(item);
	case "binary": return fromHex(item).buffer;
	case "datetime":
		// a Date holds the instant only; the zone and the fields come from
		// the machine
		return new Date(Number(item.ts) * 1000);
	case "array": return item.map(native);
	case "struct": {
		let result = {};
		for (let [name, member] of item) { result[text(name)] = native(member); }
		return result;
	}
	}
	throw new Error(`unknown value ${tag}`);
}

function version(revision) {
	if (revision == "2.1") { return 2; }
	if (revision == "3.0") { return 3; }
	throw new UnsupportedRevision(`the module encodes 2.1 and 3.0 only`);
}

function encode(message, revision) {
	let options = { version: version(revision) };
	let [[tag, item]] = Object.entries(message);
	let bytes;
	if (tag == "call") {
		bytes = frpc.serializeCall(text(item.name),
		                           item.params.map(native), undefined, options);
	} else if (tag == "response") {
		let header = [0xCA, 0x11].concat(revision.split(".").map(Number));
		bytes = header.concat([14 << 3],
		                      frpc.serialize(native(item), undefined, options));
	} else if (tag == "fault") {
		throw new Error("the module cannot encode a fault");
	} else {
		throw new Error(`unknown message ${tag}`);
	}
	// callers send the array as a Uint8Array, which truncates what is not
	// an octet (e.g. the fractional zone of a date before 1600)
	return toHex(new Uint8Array(bytes));
}

// requests

function info() {
	return { name: NAME, version: VERSION,
	         decode: ["1.0", "2.0", "2.1", "3.0"],
	         encode: ["2.1", "3.0"], options: {} };
}

function handle(line) {
	let answer = {};
	try {
		let request = JSON.parse(line);
		if (request.op == "info") { return info(); }
		answer.id = request.id;
		if (request.op == "decode") {
			answer.value = decode(fromHex(request.hex));
		} else if (request.op == "encode") {
			answer.hex = encode(request.value, request.revision);
		} else if (request.op == "relay") {
			answer.hex = encode(decode(fromHex(request.hex)), request.to);
		} else {
			throw new Error(`unknown op ${request.op}`);
		}
	} catch (e) {
		let unsupported = e instanceof UnsupportedRevision
			|| /^Unsupported FRPC version/.test(e && e.message);
		answer.error = e instanceof Unrepresentable ? "unrepresentable"
			: unsupported ? "unsupported-revision"
			: "malformed";
		answer.detail = String(e && e.message || e);
	}
	return answer;
}

for await (const line of readline.createInterface({ input: process.stdin })) {
	process.stdout.write(JSON.stringify(handle(line)) + "\n");
}
