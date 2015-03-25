#!/bin/sh

# http://wiki.nginx.org/Modules

./configure \
    --conf-path=/etc/nginx/nginx.conf \
    --error-log-path=/var/log/nginx/error_challenge.log \
    --http-client-body-temp-path=/var/lib/nginx/body \
    --http-log-path=/var/log/nginx/access_challenge.log \
    --lock-path=/var/lock/nginx_challenge.lock \
    --pid-path=/var/run/nginx_challenge.pid \
    --without-http_proxy_module \
    --without-http_scgi_module \
    --without-http_ssi_module \
    --without-http_split_clients_module \
    --without-http_upstream_ip_hash_module \
    --without-http_userid_module \
    --without-http_uwsgi_module \
    --without-http_referer_module \
    --without-http_memcached_module \
    --without-http_map_module \
    --without-http_autoindex_module \
    --without-http_access_module \
    --without-http_auth_basic_module \
    --without-http_browser_module \
    --without-http_charset_module \
    --without-http_empty_gif_module \
    --without-http_fastcgi_module \
    --without-http_geo_module \
    --without-http_gzip_module \
    --with-http_stub_status_module \
    --with-http_secure_link_module
