#!/bin/bash

case "$1" in
  add)
    docker exec -it ntfy ntfy user add "$2"
    ;;
  list)
    docker exec ntfy ntfy user list
    ;;
  delete)
    docker exec ntfy ntfy user del "$2"
    ;;
  password)
    docker exec -it ntfy ntfy user change-pass "$2"
    ;;
  access)
    # ntfy-user.sh access marco mytopic rw
    docker exec ntfy ntfy access "$2" "$3" "$4"
    ;;
  access-list)
    docker exec ntfy ntfy access list
    ;;
  token)
    docker exec ntfy ntfy token add "$2"
    ;;
  token-list)
    docker exec ntfy ntfy token list
    ;;
  backup)
    cp ~/ntfy/cache/auth.db ~/ntfy/auth.db.backup.$(date +%Y%m%d)
    echo "Backup: ~/ntfy/auth.db.backup.$(date +%Y%m%d)"
    ;;
  *)
    echo "Usage:"
    echo "  $0 add <username>          - Add user"
    echo "  $0 list                    - List users"
    echo "  $0 delete <username>       - Delete user"
    echo "  $0 password <username>     - Change password"
    echo "  $0 access <user> <topic> <rw|ro|wo> - Set access"
    echo "  $0 access-list             - List access rules"
    echo "  $0 token <username>        - Create token"
    echo "  $0 token-list              - List tokens"
    echo "  $0 backup                  - Backup auth.db"
    ;;
esac
