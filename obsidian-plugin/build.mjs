import esbuild from "esbuild";
import { readFileSync } from "fs";

const watch = process.argv.includes("--watch");


const ctx = await esbuild.context({
  entryPoints: ["src/main.js"],
  bundle: true,
  outfile: "dist/main.js",
  platform: "node",
  format: "cjs",
  target: "es2020",
  external: ["obsidian", "electron"],
  logLevel: "info",
  sourcemap: "inline",
  plugins: [],
});

if (watch) {
  await ctx.watch();
  console.log("Watching for changes...");
} else {
  await ctx.rebuild();
  await ctx.dispose();
}
