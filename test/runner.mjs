#!/usr/bin/env node

import * as fastrpc from "../javascript/fastrpc.mjs";
import * as fs from "fs";

const TESTS = "frpc.tests";

function LEX_CMP(a, b) { return a.localeCompare(b); }

function serialize(value) {
	if (value === null) { return "null"; }
	if (value instanceof Array) {
		return `(${value.map(serialize).join(", ")})`;
	}
	if (value instanceof Uint8Array) {
		let bytes = [];
		value.forEach(b => bytes.push(b.toString(16).padStart(2, "0").toLowerCase()));
		return `b"${bytes.join(" ")}"`;
	}
	if (value instanceof Date) {
		let offset = value.getTimezoneOffset();
		let h = value.getHours() + offset/60;
		let d = `${value.getFullYear()}${(value.getMonth()+1).toString().padStart(2, "0")}${value.getDate()}`;
		let t = `${h}:${value.getMinutes()}:${value.getSeconds()}+0000`;
		return `${d}T${t}`;
	}

	switch (typeof(value)) {
		case "string": return `"${value}"`; break;
		case "object":
			let keys = Object.keys(value).sort(LEX_CMP);
			let pairs = keys.map(key => `${key}: ${serialize(value[key])}`);
			return `{${pairs.join(", ")}}`;
		break;
		case "number":
			if (isNaN(value)) { return "nan"; }
			if (value == Infinity) { return "inf"; }
			if (value == -Infinity) { return "-inf"; }
			if (1/value == -Infinity) { return "-0"; }
			if (Math.trunc(value) != value) {
				return value.toFixed(6);
			} else {
				return value.toString();
			}
		break;
		case "boolean": return value.toString(); break;

		default: throw new Error(`Unserializable value ${typeof(value)}`); break;
	}
}

function assert(label, value, expected = true) {
	if (value != expected) { throw new Error(`${label}: ${value} should equal ${expected}`); }
}

function evaluate(test, result) {
//	console.log(result);
	const e = test.expectation;
	switch (e.type) {
		case "error": assert(`${test.label}: should throw`, result instanceof Error); break;
		case "fault":
			assert(`${test.label}: should throw fault exception`, result instanceof Error);
			let msg = result.message.replace(":", ",").replace("FRPC/", "");
			assert(`${test.label}: fault code/message`, msg, e.value);
		break;
		case "value":
			assert(`${test.label}: should not throw`, result instanceof Error, false);
			let s = serialize(result);
			assert(test.label, s, e.value);
		break;

		default:
			assert(`${test.label}: method name`, result.method, e.type);
			assert(`${test.label}: method params`, serialize(result.params[0]), e.value);
		break;
	}
}

function execute(test, parse) {
	let result;
	try {
		result = parse(test.data);
	} catch (e) {
		result = e;
	}

	evaluate(test, result);
}

function stringToHex(str) {
	return str.split("").map(ch => ch.charCodeAt(0).toString(16).padStart(2, "0")).join("");
}

function parseTestData(str) {
	str = str.replace(/"(.*?)"/g, (all, str) => stringToHex(str));
	str = str.replace(/\s*/g, "");

	let bytes = [];
	let chars = str.split("");
	while (chars.length) {
		let hex = `${chars.shift()}${chars.shift()}`;
		bytes.push(parseInt(hex, 16));
	}

	return bytes;
}

function parseExpectation(str) {
	let r = str.match(/^\s*([a-z]+)\((.*)\)/);
	if (r) {
		return {type:r[1], value: r[2]};
	} else {
		return {type:"value", value: str};
	}
}

function readTests() {
	let raw = fs.readFileSync(TESTS).toString("utf-8");
	let lines = raw.split("\n").filter(line => !line.match(/^\s*#/)).filter(line => line.trim());

	let results = [];

	let label = null;
	let data = null;
	let expectation = null;

	while (lines.length) {
		let line = lines.shift();
		let r = line.match(/^\s*@(.*)/);
		if (r) {
			label = r[1];
		} else if (!data) {
			data = parseTestData(line);
		} else {
			expectation = parseExpectation(line);
			results.push({label, data, expectation});
			data = null;
			expectation = null;
		}

	}
	return results;
}

function run() {
	let tests = readTests();
//	tests = tests.filter(t => t.label == "binary v1 bad size");

	let executed = 0;
	let passed = 0;
	let failed = 0;
	let errors = [];

	let parse = str => fastrpc.parse(str, {typedArrays:true});

	while (tests.length) {
		let test = tests.shift();
		try {
			execute(test, parse);
			passed++;
			process.stdout.write(".");
		} catch (e) {
			errors.push(e);
			failed++;
			process.stdout.write("E");
		}
		executed++;
	}
	process.stdout.write("\n");

	errors.forEach(e => console.log(e.message));

	console.log(executed, passed, failed);
}


run();