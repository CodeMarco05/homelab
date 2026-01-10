#!/bin/bash

CONTAINER="ntfy"
CACHE_DB="/var/cache/ntfy/cache.db"

# Farben für Output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

show_usage() {
    echo -e "${BLUE}=== ntfy Admin Tool ===${NC}"
    echo ""
    echo "Topic Management:"
    echo "  $0 topics                      - List all topics"
    echo "  $0 topic-stats                 - Show topic statistics"
    echo "  $0 topic-messages <topic>      - Show messages for topic"
    echo "  $0 topic-delete <topic>        - Delete all messages for topic"
    echo ""
    echo "Message Management:"
    echo "  $0 messages                    - Show recent messages"
    echo "  $0 messages <topic>            - Show messages for specific topic"
    echo "  $0 message-delete <id>         - Delete specific message by ID"
    echo "  $0 message-delete-old <days>   - Delete messages older than X days"
    echo "  $0 message-count               - Count total messages"
    echo ""
    echo "Cache Management:"
    echo "  $0 cache-size                  - Show cache database size"
    echo "  $0 cache-clean                 - Clean old messages (30+ days)"
    echo "  $0 cache-vacuum                - Vacuum database (optimize)"
    echo "  $0 cache-backup                - Backup cache database"
    echo ""
    echo "Search & Stats:"
    echo "  $0 search <keyword>            - Search messages"
    echo "  $0 stats                       - Show server statistics"
    echo "  $0 export <topic>              - Export topic as JSON"
    echo ""
}

case "$1" in
    # ===== TOPICS =====
    topics)
        echo -e "${BLUE}=== All Topics ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB \
            "SELECT DISTINCT topic FROM messages ORDER BY topic;"
        ;;
    
    topic-stats)
        echo -e "${BLUE}=== Topic Statistics ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF
.mode column
.headers on
SELECT 
    topic,
    COUNT(*) as messages,
    MIN(datetime(time, 'unixepoch', 'localtime')) as first_msg,
    MAX(datetime(time, 'unixepoch', 'localtime')) as last_msg
FROM messages 
GROUP BY topic 
ORDER BY messages DESC;
EOF
        ;;
    
    topic-messages)
        if [ -z "$2" ]; then
            echo -e "${RED}Error: Topic name required${NC}"
            echo "Usage: $0 topic-messages <topic>"
            exit 1
        fi
        echo -e "${BLUE}=== Messages for topic: $2 ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF
.mode column
.headers on
SELECT 
    id,
    datetime(time, 'unixepoch', 'localtime') as time,
    title,
    message
FROM messages 
WHERE topic='$2' 
ORDER BY time DESC 
LIMIT 20;
EOF
        ;;
    
    topic-delete)
        if [ -z "$2" ]; then
            echo -e "${RED}Error: Topic name required${NC}"
            echo "Usage: $0 topic-delete <topic>"
            exit 1
        fi
        COUNT=$(docker exec $CONTAINER sqlite3 $CACHE_DB \
            "SELECT COUNT(*) FROM messages WHERE topic='$2';")
        echo -e "${YELLOW}Warning: This will delete $COUNT messages from topic '$2'${NC}"
        read -p "Are you sure? (yes/no): " confirm
        if [ "$confirm" = "yes" ]; then
            docker exec $CONTAINER sqlite3 $CACHE_DB \
                "DELETE FROM messages WHERE topic='$2';"
            echo -e "${GREEN}Deleted $COUNT messages from topic '$2'${NC}"
        else
            echo "Cancelled"
        fi
        ;;
    
    # ===== MESSAGES =====
    messages)
        if [ -z "$2" ]; then
            echo -e "${BLUE}=== Recent Messages (Last 20) ===${NC}"
            docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF
.mode column
.headers on
SELECT 
    id,
    datetime(time, 'unixepoch', 'localtime') as time,
    topic,
    title,
    substr(message, 1, 50) as message
FROM messages 
ORDER BY time DESC 
LIMIT 20;
EOF
        else
            docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF
.mode column
.headers on
SELECT 
    id,
    datetime(time, 'unixepoch', 'localtime') as time,
    title,
    message
FROM messages 
WHERE topic='$2' 
ORDER BY time DESC 
LIMIT 20;
EOF
        fi
        ;;
    
    message-delete)
        if [ -z "$2" ]; then
            echo -e "${RED}Error: Message ID required${NC}"
            echo "Usage: $0 message-delete <id>"
            exit 1
        fi
        docker exec $CONTAINER sqlite3 $CACHE_DB \
            "DELETE FROM messages WHERE id='$2';"
        echo -e "${GREEN}Message $2 deleted${NC}"
        ;;
    
    message-delete-old)
        DAYS=${2:-30}
        COUNT=$(docker exec $CONTAINER sqlite3 $CACHE_DB \
            "SELECT COUNT(*) FROM messages WHERE time < strftime('%s', 'now', '-$DAYS days');")
        echo -e "${YELLOW}Warning: This will delete $COUNT messages older than $DAYS days${NC}"
        read -p "Are you sure? (yes/no): " confirm
        if [ "$confirm" = "yes" ]; then
            docker exec $CONTAINER sqlite3 $CACHE_DB \
                "DELETE FROM messages WHERE time < strftime('%s', 'now', '-$DAYS days');"
            echo -e "${GREEN}Deleted $COUNT old messages${NC}"
        else
            echo "Cancelled"
        fi
        ;;
    
    message-count)
        echo -e "${BLUE}=== Message Count ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB \
            "SELECT COUNT(*) as total_messages FROM messages;"
        ;;
    
    # ===== CACHE =====
    cache-size)
        echo -e "${BLUE}=== Cache Size ===${NC}"
        docker exec $CONTAINER du -sh $CACHE_DB
        docker exec $CONTAINER ls -lh $CACHE_DB
        ;;
    
    cache-clean)
        echo -e "${BLUE}=== Cleaning messages older than 30 days ===${NC}"
        COUNT=$(docker exec $CONTAINER sqlite3 $CACHE_DB \
            "SELECT COUNT(*) FROM messages WHERE time < strftime('%s', 'now', '-30 days');")
        if [ "$COUNT" -gt 0 ]; then
            docker exec $CONTAINER sqlite3 $CACHE_DB \
                "DELETE FROM messages WHERE time < strftime('%s', 'now', '-30 days');"
            echo -e "${GREEN}Deleted $COUNT old messages${NC}"
        else
            echo "No old messages to clean"
        fi
        ;;
    
    cache-vacuum)
        echo -e "${BLUE}=== Vacuuming database ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB "VACUUM;"
        echo -e "${GREEN}Database optimized${NC}"
        ;;
    
    cache-backup)
        BACKUP_FILE="cache-backup-$(date +%Y%m%d-%H%M%S).db"
        docker exec $CONTAINER sqlite3 $CACHE_DB ".backup /var/cache/ntfy/$BACKUP_FILE"
        docker cp $CONTAINER:/var/cache/ntfy/$BACKUP_FILE ~/ntfy/backups/$BACKUP_FILE 2>/dev/null || \
            docker cp $CONTAINER:/var/cache/ntfy/$BACKUP_FILE ~/ntfy/$BACKUP_FILE
        echo -e "${GREEN}Backup created: $BACKUP_FILE${NC}"
        ;;
    
    # ===== SEARCH & STATS =====
    search)
        if [ -z "$2" ]; then
            echo -e "${RED}Error: Search keyword required${NC}"
            echo "Usage: $0 search <keyword>"
            exit 1
        fi
        echo -e "${BLUE}=== Search results for: $2 ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF
.mode column
.headers on
SELECT 
    datetime(time, 'unixepoch', 'localtime') as time,
    topic,
    title,
    message
FROM messages 
WHERE message LIKE '%$2%' OR title LIKE '%$2%' 
ORDER BY time DESC 
LIMIT 20;
EOF
        ;;
    
    stats)
        echo -e "${BLUE}=== ntfy Statistics ===${NC}"
        docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF
SELECT 'Total Messages: ' || COUNT(*) FROM messages;
SELECT 'Total Topics: ' || COUNT(DISTINCT topic) FROM messages;
SELECT 'Oldest Message: ' || MIN(datetime(time, 'unixepoch', 'localtime')) FROM messages;
SELECT 'Newest Message: ' || MAX(datetime(time, 'unixepoch', 'localtime')) FROM messages;
EOF
        echo ""
        docker exec $CONTAINER du -sh /var/cache/ntfy
        ;;
    
    export)
        if [ -z "$2" ]; then
            echo -e "${RED}Error: Topic name required${NC}"
            echo "Usage: $0 export <topic>"
            exit 1
        fi
        OUTPUT_FILE="export-$2-$(date +%Y%m%d).json"
        docker exec $CONTAINER sqlite3 $CACHE_DB <<EOF > ~/ntfy/$OUTPUT_FILE
.mode json
SELECT * FROM messages WHERE topic='$2' ORDER BY time DESC;
EOF
        echo -e "${GREEN}Exported to: ~/ntfy/$OUTPUT_FILE${NC}"
        ;;
    
    *)
        show_usage
        ;;
esac
