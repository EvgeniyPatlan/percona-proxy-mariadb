#!/bin/bash
#
# Renames MariaDB MaxScale to Percona Proxy for MariaDB across the tree.
#
# The rename is done once, when the fork is created, and this script is kept so that the
# change is reproducible and reviewable. It is not meant to be run again on an already
# renamed tree.
#
# Two spellings are produced, and which one applies depends on the context:
#
#   percona_proxy   C and C++ identifiers, macros and namespaces, where a hyphen is not legal
#   percona-proxy   file names, paths, configuration keys, package names and anything a user sees
#
# The internal mxs::, mxb:: namespaces and the MXS_ macros are deliberately left alone: they are
# invisible to users and renaming them would multiply the size of the change for no benefit.
#
# Usage: tools/rename.sh [--dry-run]
#

set -o errexit
set -o pipefail

DRY_RUN=0
[ "$1" = "--dry-run" ] && DRY_RUN=1

cd "$(dirname "$0")/.."

# Third-party code keeps its own names, and the licensing files keep the upstream wording.
EXCLUDE_PATHS=(
    './.git/*'
    './mariadb-connector-c/*'
    './jwt-cpp/*'
    './server/modules/parser_plugin/pp_sqlite/sqlite-src-*'
    './licenses/*'
    './COPYING'
    './NOTICE'
    './LICENSE.TXT'
    './LICENSE-THIRDPARTY.TXT'
    # The rename tooling itself: a sed pass over these files would rewrite the rules.
    './tools/*'
    # The Percona packaging, container images and build scripts reference the upstream package
    # name deliberately (Conflicts, Provides, Obsoletes) and are renamed by hand when the
    # packaging is reworked, so a mechanical rename would corrupt them.
    './BUILD/percona/*'
)

find_args=(. -type f)
rename_excludes=()
for path in "${EXCLUDE_PATHS[@]}"
do
    find_args+=(! -path "$path")
    # The file renames skip the same paths, so an excluded directory keeps both its contents
    # and its file names. Renaming only the names would leave BUILD/percona referring to files
    # that no longer exist.
    rename_excludes+=(! -path "$path")
done

# The order matters: the specific names have to be rewritten before the generic ones.
# Each entry is a sed expression.
RULES=(
    # The CMake project name has to stay a single word: project(Percona Proxy) would make
    # CMake read "Proxy" as a language to enable.
    's/project(MaxScale)/project(PerconaProxy)/g'

    # Product names in prose, before the bare name is touched.
    's/MariaDB MaxScale/Percona Proxy for MariaDB/g'
    's/MaxScale™/Percona Proxy/g'
    's/MaxGUI/Percona Proxy GUI/g'
    's/MaxCtrl/Percona Proxyctl/g'

    # Include paths and the public header directory.
    's|#include <maxscale/|#include <percona-proxy/|g'
    's|#include "maxscale/|#include "percona-proxy/|g'
    's|include/maxscale|include/percona-proxy|g'

    # C++ namespace and the macros. mxs:: keeps working: it is an alias of this namespace.
    's/\bnamespace maxscale\b/namespace percona_proxy/g'
    's/\bmaxscale::/percona_proxy::/g'
    # Namespace aliases such as "namespace mxs = maxscale;" need the identifier spelling.
    's/\(namespace [A-Za-z_][A-Za-z0-9_]* *= *\)maxscale *;/\1percona_proxy;/g'
    's/PERCONA_PROXYCTL_/PERCONA_PROXYCTL_/g'
    's/MAXGUI_/GUI_/g'
    's/\bMAXSCALE_/PERCONA_PROXY_/g'

    # Executables and helper scripts, which are hyphenated on disk.
    's/\bmaxscale_pam_auth_tool\b/percona-proxy-pam-auth-tool/g'
    's/\bmaxscale_generate_support_info\.py\b/percona-proxy-generate-support-info.py/g'
    's/\bmaxscale_logrotate\b/percona-proxy-logrotate/g'
    's/\bmaxscale_test\.cnf\b/percona-proxy-test.cnf/g'
    's/\bmaxscale_test\.h\b/percona_proxy_test.h/g'
    # No word boundaries here: identifiers such as maxctrl_node_modules must be caught too.
    's/maxctrl/percona-proxyctl/g'
    's/maxkeys/percona-proxy-keys/g'
    's/maxpasswd/percona-proxy-passwd/g'
    's/maxavrocheck/percona-proxy-avrocheck/g'
    's/gui/gui/g'

    # Remaining C identifiers, for example maxscale_commit() or maxscale_started.
    's/\bmaxscale_/percona_proxy_/g'
    # Source files such as mariadb_maxscale.cc, where the name follows an underscore.
    's/_maxscale\b/_percona_proxy/g'

    # CMake targets and libraries.
    's/\bmaxscale-common\b/percona-proxy-common/g'
    's/\blibmaxscale-common\b/libpercona-proxy-common/g'

    # Names where the old one is glued to a suffix, so the bare rules below (which need a word
    # boundary) never see them. A hyphen cannot be used here: these are C++, Java and shell
    # identifiers, where "percona-proxy" would parse as a subtraction.
    # The plural has to come first, or it turns into "percona_proxy_s".
    # No leading \b: an underscore is a word character, so "\bmaxscalepcre2" would not match
    # inside test_maxscalepcre2.
    's/maxscales\b/percona_proxies/g'
    's/maxscales_/percona_proxies_/g'
    's/MaxScales\b/PerconaProxies/g'
    's/maxscalehost\b/percona_proxy_host/g'
    's/maxscaledir\b/percona_proxy_dir/g'
    's/maxscaleuser\b/percona_proxy_user/g'
    's/maxscalepcre2\b/percona_proxy_pcre2/g'
    's/maxscale\([0-9]\)/percona_proxy\1/g'
    # PascalCase class and file names: MaxScaleConnection, MaxScaleParameters, MaxScale1.
    # Lower-case camelCase (maxscaleSetVariable and friends) is deliberately not matched here;
    # it belongs to the bundled SQLite grammar and is restored at the end.
    's/\bMaxScale\([A-Za-z0-9_]\)/PerconaProxy\1/g'

    # Everything left: paths, configuration section, REST endpoint, package and service names,
    # and the bare product name in prose.
    's/\bmaxscale\b/percona-proxy/g'
    # Upstream spells the name both ways in prose.
    's/\bMaxscale\b/Percona Proxy/g'
    's/\bMaxScale\b/Percona Proxy/g'
    's/\bMAXSCALE\b/PERCONA_PROXY/g'

    # Restore the names shared with the bundled SQLite tree, which is deliberately left
    # unrenamed (see the exclusions above). Its configure script only understands
    # --enable-maxscale, and its grammar calls these functions, which pp_sqlite.cc defines.
    # Renaming only one side of that boundary breaks the grammar and then the link.
    # SQL user variables: "SET @percona-proxy.cache.populate=true" would parse as a subtraction,
    # so the session variables the filters register keep an underscore.
    's/@percona-proxy\./@percona_proxy./g'

    's/--enable-percona-proxy/--enable-maxscale/g'
    's/\bpercona_proxy_create_pseudo_limit\b/maxscale_create_pseudo_limit/g'
    's/\bpercona_proxy_set_type_mask\b/maxscale_set_type_mask/g'
    's/\bpercona_proxy_update_function_info\b/maxscale_update_function_info/g'
)

# Identifiers need percona_proxy, not percona-proxy, so they are rewritten first, while the
# code can still be told apart from the strings and comments around it. In a Vue component
# only the <script> block is treated as code.
echo "== renaming identifiers in sources"
mapfile -t code_files < <(find "${find_args[@]}" \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' \
    -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' \
    -o -name '*.js' -o -name '*.mjs' -o -name '*.cjs' -o -name '*.py' -o -name '*.vue' \) \
    ! -path '*/node_modules/*' -print0 \
    | xargs -0 grep -lIE 'maxscale|MaxScale|Percona Proxy|MAXSCALE|maxctrl|MaxCtrl|gui|MaxGUI|maxkeys|maxpasswd|maxavrocheck' 2>/dev/null || true)
echo "   ${#code_files[@]} sources contain the name"

if [ "$DRY_RUN" = 0 ] && [ "${#code_files[@]}" -gt 0 ]
then
    printf '%s\0' "${code_files[@]}" | xargs -0 python3 tools/rename_identifiers.py
fi

echo "== rewriting file contents"
mapfile -t files < <(find "${find_args[@]}" -print0 | xargs -0 grep -lIE 'maxscale|MaxScale|Percona Proxy|MAXSCALE|maxctrl|MaxCtrl|gui|MaxGUI|maxkeys|maxpasswd|maxavrocheck' 2>/dev/null || true)
echo "   ${#files[@]} files contain the name"

if [ "$DRY_RUN" = 0 ]
then
    sed_script=$(printf '%s;' "${RULES[@]}")
    printf '%s\0' "${files[@]}" | xargs -0 -P "$(nproc)" -n 50 sed -i "$sed_script"
fi

echo "== renaming files and directories"
# Deepest paths first, so that renaming a directory does not invalidate the paths below it.
rename_one() {
    local src=$1 dst=$2
    [ "$src" = "$dst" ] && return 0
    if [ "$DRY_RUN" = 1 ]
    then
        echo "   $src -> $dst"
        return 0
    fi
    # mv moves into a directory that already exists instead of failing, which would bury the
    # tree one level deeper (gui/gui) and leave the build looking for a CMakeLists.txt that
    # is no longer where it was. Refuse instead: on a re-run, clean the tree with
    # "git clean -fdx" first, because an ignored file (.vscode) keeps the old directory alive.
    if [ -e "$dst" ]
    then
        echo "   error: $dst already exists, refusing to move $src onto it" >&2
        exit 1
    fi
    mkdir -p "$(dirname "$dst")"
    git mv "$src" "$dst" 2>/dev/null || mv "$src" "$dst"
}

while IFS= read -r path
do
    base=$(basename "$path")
    dir=$(dirname "$path")
    new=$(echo "$base" \
        | sed -e 's/_maxscale\./_percona_proxy./' \
              -e 's/maxscale_test\.h\.in/percona_proxy_test.h.in/' \
              -e 's/maxscale_test\.cnf/percona-proxy-test.cnf/' \
              -e 's/maxscale_generate_support_info\.py/percona-proxy-generate-support-info.py/' \
              -e 's/maxscale_pam_auth_tool/percona-proxy-pam-auth-tool/' \
              -e 's/maxscale_logrotate/percona-proxy-logrotate/' \
              -e 's/maxscalepcre2/percona_proxy_pcre2/g' \
              -e 's/maxscales/percona_proxies/g' \
              -e 's/MaxScales/PerconaProxies/g' \
              -e 's/maxscale_/percona_proxy_/' \
              -e 's/maxctrl/percona-proxyctl/g' \
              -e 's/maxkeys/percona-proxy-keys/g' \
              -e 's/maxpasswd/percona-proxy-passwd/g' \
              -e 's/maxavrocheck/percona-proxy-avrocheck/g' \
              -e 's/gui/gui/g' \
              -e 's/MaxScale\([A-Za-z0-9_]\)/PerconaProxy\1/g' \
              -e 's/MaxScale/Percona-Proxy/g' \
              -e 's/MaxCtrl/Percona-Proxyctl/g' \
              -e 's/MaxGUI/Percona-Proxy-GUI/g' \
              -e 's/maxscale/percona-proxy/g')
    rename_one "$path" "$dir/$new"
done < <(find . -depth \( -iname '*maxscale*' -o -iname '*maxctrl*' -o -iname '*gui*' \
    -o -iname '*maxkeys*' -o -iname '*maxpasswd*' -o -iname '*maxavrocheck*' \) \
    "${rename_excludes[@]}")

echo "== done"
echo "Remaining mentions (expected: third-party code, licenses, provenance):"
find "${find_args[@]}" -print0 | xargs -0 grep -lI 'maxscale\|MaxScale' 2>/dev/null | head -20 || true
