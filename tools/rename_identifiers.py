#!/usr/bin/env python3
"""Rename the product name where it is an identifier rather than text.

The rest of the rename is done with sed by tools/rename.sh, which cannot tell code from the
text around it. That matters because the two need different spellings: a path, a
configuration section or a sentence becomes "percona-proxy", but no language here allows a
hyphen in an identifier, so "json_t* maxscale" has to become "json_t* percona_proxy", and
"async function maxctrl()" has to become "async function percona_proxyctl()".

This pass therefore rewrites only the code, leaving string literals, comments and #include
lines for sed. It runs before sed, on the original spelling.
"""
import re
import sys

# Each entry is a regular expression and its replacement, tried in order, so the plural and
# the underscore-joined forms come before the bare name they start with.
#
# A trailing \b is what protects the camelCase maxscaleSetVariable family, which the bundled
# SQLite grammar calls and which therefore keeps the old spelling on both sides of that
# boundary. The forms ending in an underscore carry no trailing \b, so an identifier such as
# close_maxscale_connections is renamed as well; the three maxscale_ functions shared with
# that grammar are restored afterwards by tools/rename.sh.
RENAMES = [
    (r"maxscales_", "percona_proxies_"),
    (r"maxscales\b", "percona_proxies"),
    (r"MaxScales\b", "PerconaProxies"),
    (r"Maxscales\b", "PerconaProxies"),
    (r"maxscale_", "percona_proxy_"),
    (r"maxscale\b", "percona_proxy"),
    (r"MaxScale_", "PerconaProxy_"),
    (r"MaxScale\b", "PerconaProxy"),
    (r"Percona Proxy\b", "PerconaProxy"),
    # maxctrl needs no boundary at all: maxctrlf is a real method name, and unlike maxscale
    # there is no camelCase family here that has to keep the old spelling.
    (r"maxctrl", "percona_proxyctl"),
    (r"MaxCtrl", "PerconaProxyctl"),
]
TOKENS = re.compile("|".join("(" + pattern + ")" for pattern, _ in RENAMES))
REPLACEMENTS = [replacement for _, replacement in RENAMES]

# MAXSCALE is deliberately absent: it maps to PERCONA_PROXY everywhere, so sed can do it.

# What is *not* code, per language. The order matters: the longer quote has to come first,
# or the shorter one matches its first two characters as an empty string.
_C_STRINGS = [
    r'R"([^(\s]*)\(.*?\)\1"',                   # raw string, keeping its own delimiter
    r'"(?:\\.|[^"\\])*"',
    r"'(?:\\.|[^'\\])*'",
]
NON_CODE = {
    "c": [r"^[ \t]*#[ \t]*include[^\n]*", r"//[^\n]*", r"/\*.*?\*/"] + _C_STRINGS,
    "js": [r"//[^\n]*", r"/\*.*?\*/", r"`(?:\\.|[^`\\])*`",
           r'"(?:\\.|[^"\\])*"', r"'(?:\\.|[^'\\])*'"],
    "py": [r"#[^\n]*", r'""".*?"""', r"'''.*?'''",
           r'"(?:\\.|[^"\\\n])*"', r"'(?:\\.|[^'\\\n])*'"],
}
SCANNERS = {
    language: re.compile("|".join(patterns), re.DOTALL | re.MULTILINE)
    for language, patterns in NON_CODE.items()
}
LANGUAGES = {
    ".c": "c", ".cc": "c", ".cpp": "c", ".h": "c", ".hh": "c", ".hpp": "c",
    ".js": "js", ".mjs": "js", ".cjs": "js",
    ".py": "py",
    ".vue": "vue",
}

# A Vue single-file component is a template, a script and a style in one file. Only the
# script holds identifiers; the template and the style are text, and sed handles them.
SCRIPT_BLOCK = re.compile(r"(<script\b[^>]*>)(.*?)(</script>)", re.DOTALL | re.IGNORECASE)


def _replace(match):
    return REPLACEMENTS[match.lastindex - 1]


def convert(text, language="c"):
    """Rewrite identifiers in the code of `text`, leaving strings and comments alone."""
    if language == "vue":
        return SCRIPT_BLOCK.sub(
            lambda m: m.group(1) + convert(m.group(2), "js") + m.group(3), text)
    scanner = SCANNERS[language]
    out = []
    position = 0
    for span in scanner.finditer(text):
        out.append(TOKENS.sub(_replace, text[position:span.start()]))
        out.append(span.group(0))
        position = span.end()
    out.append(TOKENS.sub(_replace, text[position:]))
    return "".join(out)


def main(paths):
    changed = 0
    for path in paths:
        language = LANGUAGES.get(path[path.rfind("."):]) if "." in path else None
        if language is None:
            continue
        with open(path, encoding="utf-8", errors="surrogateescape") as f:
            text = f.read()
        new = convert(text, language)
        if new != text:
            with open(path, "w", encoding="utf-8", errors="surrogateescape") as f:
                f.write(new)
            changed += 1
    print(f"   {changed} files had identifiers renamed")


if __name__ == "__main__":
    main(sys.argv[1:])
