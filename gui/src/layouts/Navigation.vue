<template>
    <v-navigation-drawer
        :mini-variant.sync="isMini"
        color="white"
        class="main-nav"
        width="210"
        mini-variant-width="50"
        fixed
        left
        clipped
        app
        permanent
        @mouseover.native="isMini = false"
        @mouseout.native="isMini = true"
    >
        <v-list class="pa-0">
            <nav-item
                v-for="item in topItems"
                :key="item.label"
                :item="item"
                :currentPath="currentPath"
                :isMini="isMini"
                @click.native="navigate(item)"
            />
        </v-list>
        <template v-slot:append>
            <v-list class="bottom-nav">
                <nav-item
                    v-for="item in bottomItems"
                    :key="item.label"
                    :item="item"
                    :currentPath="currentPath"
                    :isMini="isMini"
                    @click.native="navigate(item)"
                />
            </v-list>
        </template>
    </v-navigation-drawer>
</template>

<script>
/*
 * Copyright (c) 2020 MariaDB Corporation Ab
 * Copyright (c) 2023 MariaDB plc, Finnish Branch
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2026-09-21
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */
import { mapState } from 'vuex'
import NavItem from '@rootSrc/layouts/NavItem.vue'
import { sideBarRoutes } from '@rootSrc/router/routes'

export default {
    name: 'navigation',
    components: { NavItem },
    data() {
        return {
            isMini: true,
        }
    },
    computed: {
        ...mapState({
            percona_proxy_version: state => state.percona_proxy.percona_proxy_version,
        }),
        currentPath() {
            return this.$route.path
        },
        topItems() {
            return sideBarRoutes.filter(item => !item.meta.isBottom)
        },
        bottomItems() {
            return sideBarRoutes.filter(item => item.meta.isBottom)
        },
    },

    methods: {
        navigate(nxtRoute) {
            const { path, meta } = nxtRoute
            if (meta.external) {
                let url = meta.external
                if (url === 'document') {
                    // The documentation ships with the source; there is no knowledge base
                    // keyed by version to derive a URL from.
                    url =
                        'https://github.com/EvgeniyPatlan/percona-proxy-mariadb/tree/main/Documentation'
                }
                window.open(url, '_blank', 'noopener,noreferrer')
            } else {
                /**
                 * E.g. Sidebar dashboard path is /dashboard, but it'll be redirected to /dashboard/servers
                 * This checks it and prevent redundant navigation
                 */
                const isDupRoute = this.currentPath.includes(meta.redirect || path)
                if (path && path !== this.currentPath && !isDupRoute) {
                    this.$router.push(path)
                }
            }
        },
    },
}
</script>

<style lang="scss" scoped>
.main-nav {
    z-index: 7;
    ::v-deep.v-navigation-drawer__border {
        background-color: $separator !important;
    }
}
</style>
