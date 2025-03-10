#!/usr/bin/env bun
import * as BMP from "bmp-js";
import * as fs from "fs";
import * as path from "path";

const PROGRAM_NAME = "bmp-to-xbm-c.mjs";

function main() {
  const argv = process.argv.slice(2);
  const [filePathRaw, varBase = "img"] = argv;

  if (!filePathRaw) {
    throw new Error(`usage: ${PROGRAM_NAME} <filename> [var_base]`);
  }
  const filePath = path.resolve(filePathRaw);
  const bmpBytes = fs.readFileSync(filePath);

  const xbmImage = bmpToXBM(bmpBytes);
  const cCode = bytesToCCode(xbmImage, varBase, filePath);
  console.log(cCode);
}

/** @typedef {{ width: number, height: number, data: Buffer }} XBMImage */

/** @returns {XBMImage} */
function bmpToXBM(/** @type {Buffer} */ bmpData) {
  const bmp = BMP.decode(bmpData);

  const bytes = [];
  let currentByte = 0b00000000;
  let bitOffset = 0;
  for (let i = 0; i < bmp.data.length; i += 4) {
    const pixel = bmp.data.readUInt32LE(i);
    const isPixelOn = pixel !== 0;

    if (isPixelOn) {
      // the lowest bit is the leftmost pixel
      // . . . . . . . .
      // 0 1 2 3 4 5 6 7
      currentByte |= 1 << bitOffset;
    }
    if (bitOffset === 7) {
      bytes.push(currentByte);
      currentByte = 0b00000000;
      bitOffset = 0;
    } else {
      bitOffset++;
    }
  }
  if (bytes.length !== (bmp.width * bmp.height) / 8) {
    throw new Error(
      `Expected ${(width * height) / 8} bytes, got ${bytes.length} bytes`
    );
  }
  return { width: bmp.width, height: bmp.height, data: bytes };
}

const BYTES_PER_ROW = 19;
const INDENT = "    ";

/** @returns {string} */
function bytesToCCode(
  /** @type {XBMImage} */ xbmImage,
  /** @type {string} */ varBase,
  /** @type {string} */ sourceFilePath
) {
  const { width, height, data } = xbmImage;
  let res = [];
  res.push(`#include <U8g2lib.h>`);
  res.push(``);
  res.push(
    `// generated from '${path.basename(
      sourceFilePath
    )}' using \`${PROGRAM_NAME}\``
  );
  res.push(`#define ${varBase}_width ${width}`);
  res.push(`#define ${varBase}_height ${height}`);

  // note: the U8X8_PROGMEM is important, we don't want the imge to be in RAM
  res.push(`static const unsigned char ${varBase}_bits[] U8X8_PROGMEM = {`);
  {
    const pushRow = (row) => {
      res.push(
        INDENT +
          row
            .map((byte) => {
              return `0x${byte.toString(16).padStart(2, "0").toUpperCase()}`;
            })
            .join(", ") +
          ","
      );
    };
    let row = [];
    for (const byte of data) {
      row.push(byte);
      if (row.length === BYTES_PER_ROW) {
        pushRow(row);
        row = [];
      }
    }
    if (row.length > 0) {
      pushRow(row);
    }
  }

  res.push(`};`);
  return res.join("\n");
}

main();
