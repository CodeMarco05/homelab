# Traefik

Here lies the config for traefik.

## Base config docker-compose file

Create a directory for the docker compose for traefik. For example `mkdir traefik`, then create the
`docker-compose.yml` in there. The config files are also down below.

```yml
# docker-compose.yml
services:
  traefik:
    image: traefik:v3.6 # !important update traefik
    container_name: traefik
    restart: unless-stopped
    ports:
      - "80:80"
      - "443:443"
      - "4317:4317" #grpc
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock:ro
      - ./traefik.yml:/etc/traefik/traefik.yml:ro # config file for traefik -> change if needed
      - ./letsencrypt:/letsencrypt
      - ./logs:/logs
    labels:
      - "traefik.enable=true"
      - "traefik.http.routers.dashboard.rule=Host(`<domain-for-traefik-admin-dashboard>`)"
      - "traefik.http.routers.dashboard.entrypoints=websecure"
      - "traefik.http.routers.dashboard.tls.certresolver=letsencrypt"
      - "traefik.http.routers.dashboard.service=api@internal"
      - "traefik.http.routers.dashboard.middlewares=auth"
      # this can be removed if the auth is not needed for the traefik dashboard
      - "traefik.http.middlewares.auth.basicauth.users=admin:<hash-for-basic-auth>"
    networks:
      # the traefik network in which all docker containers
      # need to be for the labels to be recognised
      - traefik
    logging:
      driver: "json-file"
      options:
        max-size: "50m"
        max-file: "3"

  # dashboard for traefik
  goaccess:
    image: xavierh/goaccess-for-nginxproxymanager:latest
    container_name: goaccess
    restart: unless-stopped
    environment:
      - TZ=Europe/Berlin
      - LOG_TYPE=TRAEFIK
      - SKIP_ARCHIVED_LOGS=False
    volumes:
      - ./logs:/opt/log:ro
    labels:
      - "traefik.enable=true"
      - "traefik.http.routers.goaccess.rule=Host(`<domain-for-dashboard>`)"
      - "traefik.http.routers.goaccess.entrypoints=websecure"
      - "traefik.http.routers.goaccess.tls.certresolver=letsencrypt"
      - "traefik.http.routers.goaccess.middlewares=auth"
      - "traefik.http.services.goaccess.loadbalancer.server.port=7880"
    networks:
      - traefik

networks:
  traefik:
    name: traefik
    driver: bridge
```

## traefik.yml file

Here lies the config for traefik. This can be also done via the start command in the docker run
section but this is in my opinion more clear.

```yml
global:
  checkNewVersion: false
  # if you are nice you can turn this on
  sendAnonymousUsage: false

api:
  dashboard: true

log:
  level: INFO
  format: json
  filePath: /logs/traefik.log
  maxSize: 100
  maxAge: 7
  maxBackups: 3
  compress: true

accessLog:
  filePath: /logs/access.log
  # can be set to json but then the admin dashboard (goaccess) won't work
  format: common
  bufferingSize: 100

entryPoints:
  # the entry point for port 80
  web:
    address: ":80"
    http:
      # perma redirect to https
      redirections:
        entrypoint:
          to: websecure
          scheme: https
    # needed to forward the headers for access to be able to view
    # them if traffic is proxied through cloudflare
    # check if the ips are still right
    forwardedHeaders:
      trustedIPs: &cloudflare-ips
        - "173.245.48.0/20"
        - "103.21.244.0/22"
        - "103.22.200.0/22"
        - "103.31.4.0/22"
        - "141.101.64.0/18"
        - "108.162.192.0/18"
        - "190.93.240.0/20"
        - "188.114.96.0/20"
        - "197.234.240.0/22"
        - "198.41.128.0/17"
        - "162.158.0.0/15"
        - "104.16.0.0/13"
        - "104.24.0.0/14"
        - "172.64.0.0/13"
        - "131.0.72.0/22"
        - "2400:cb00::/32"
        - "2606:4700::/32"
        - "2803:f800::/32"
        - "2405:b500::/32"
        - "2405:8100::/32"
        - "2a06:98c0::/29"
        - "2c0f:f248::/32"

  # websecure https entrypoint
  websecure:
    address: ":443"
    forwardedHeaders:
      trustedIPs: *cloudflare-ips

  # if you need a grpc entry point for metrics, logs etc.
  grpc:
    address: ":4317"

# certificate resolver over http only works when a public IP is available
certificatesResolvers:
  letsencrypt:
    acme:
      email: <your-mail>
      storage: /letsencrypt/acme.json
      httpChallenge:
        entryPoint: web

# here can also be added more providers
providers:
  docker:
    exposedByDefault: false
```

## The setup for servers without a public IP

Here not a lot of adjustment is needed. In the `docker-compose.yml` add this as an environment

```yml
# docker-compose.yml
environment:
  - CF_DNS_API_TOKEN=${CF_DNS_API_TOKEN}
```

Then create a `.env` file where we can save the API token from cloudflare. It is important that the
token has access to DNS where edit and read is allowed.

```env
# .env
CF_DNS_API_TOKEN=<your-token>
```

Now also change the certresolver

```yml
certificatesResolvers:
  letsencrypt:
    acme:
      email: <your-mail> 
      storage: /letsencrypt/acme.json
      dnsChallenge:
        provider: cloudflare # this is important
        resolvers:
          - "1.1.1.1:53"
          - "8.8.8.8:53"
```
