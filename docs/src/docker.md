# Docker Services

Most services run as Docker Compose stacks. Each stack lives in its own directory under `/opt/stacks/` on the host, with a consistent structure.

## Directory Layout

```
/opt/stacks/
├── traefik/
│   ├── docker-compose.yml
│   ├── traefik.yml
│   └── dynamic/
├── authentik/
│   └── docker-compose.yml
├── monitoring/
│   └── docker-compose.yml
└── ...
```

Every stack uses a shared external Docker network called `proxy` so Traefik can reach containers across stacks.

## Shared Network

Create it once on the host — all stacks reference it as external:

```bash
docker network create proxy
```

In each `docker-compose.yml`:

```yaml
networks:
  proxy:
    external: true
```

## Traefik

Traefik handles all ingress. TLS certificates are issued automatically via Let's Encrypt DNS challenge through Cloudflare.

```yaml
services:
  traefik:
    image: traefik:v3.1
    restart: unless-stopped
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock:ro
      - ./traefik.yml:/traefik.yml:ro
      - ./dynamic:/dynamic:ro
      - ./certs:/certs
    networks:
      - proxy
```

Expose a service by adding labels:

```yaml
labels:
  - "traefik.enable=true"
  - "traefik.http.routers.myapp.rule=Host(`myapp.home.lab`)"
  - "traefik.http.routers.myapp.entrypoints=websecure"
  - "traefik.http.routers.myapp.tls=true"
```

## Authentik

Authentik provides SSO for all services that support OAuth2 or SAML. Services that don't natively support it use Traefik's `forwardAuth` middleware.

### Forward Auth Middleware

Define it once in a dynamic config file:

```yaml
# dynamic/authentik.yml
http:
  middlewares:
    authentik:
      forwardAuth:
        address: "http://authentik-server:9000/outpost.goauthentik.io/auth/traefik"
        trustForwardHeader: true
        authResponseHeaders:
          - X-authentik-username
          - X-authentik-groups
          - X-authentik-email
```

Then attach to any router with:

```yaml
- "traefik.http.routers.myapp.middlewares=authentik@file"
```

## Watchtower

Watchtower keeps images up to date automatically. It runs on a schedule and sends a notification to ntfy when containers are updated.

```yaml
services:
  watchtower:
    image: containrrr/watchtower
    restart: unless-stopped
    environment:
      - WATCHTOWER_SCHEDULE=0 0 4 * * *
      - WATCHTOWER_CLEANUP=true
      - WATCHTOWER_NOTIFICATION_URL=ntfy://ntfy.home.lab/watchtower
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock
```

---

See the [Kubernetes](./kubernetes.md) page for workloads that run in k3s instead of plain Docker.
