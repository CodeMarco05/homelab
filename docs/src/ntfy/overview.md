# ntfy

Push based notification service. The setup is not complete there is the traefik config missing.

## Docker compose file

```yml
{{#include ./docker-compose.yml}}
```

## Server config file

```yml
{{#include ./server.yml}}
```

## Admin script

Shell script to create admin topics etc.

```sh
{{#include ./ntfy-admin.sh}}
```

## User script

Shell script to create users

```sh
{{#include ./ntfy-user.sh}}
```
