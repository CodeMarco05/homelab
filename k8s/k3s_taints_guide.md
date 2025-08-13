# K3s Node Taints and Scheduling Guide

This guide covers how to control pod scheduling in your K3s cluster using taints, tolerations, and node affinity.

## 1. Making the Taint for Master

### Apply Standard Master Taint

```bash
# Add the standard Kubernetes master taint
kubectl taint nodes master node-role.kubernetes.io/control-plane:NoSchedule
```

### Verify the Taint

```bash
# Check that the taint was applied
kubectl describe node master | grep -i taint

# Expected output:
# Taints: node-role.kubernetes.io/control-plane:NoSchedule
```

### Check Pod Distribution

```bash
# See where pods are currently scheduled
kubectl get pods -A -o wide

# You should see:
# - System pods (kube-system) on master (they have tolerations)
# - New application pods on worker nodes only
```

## 2. Schedule Services on Any Node

### Basic Deployment (Respects Taints)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: web-app
  namespace: default
spec:
  replicas: 3
  selector:
    matchLabels:
      app: web-app
  template:
    metadata:
      labels:
        app: web-app
    spec:
      containers:
      - name: nginx
        image: nginx:alpine
        ports:
        - containerPort: 80
```

**Result:** This will only schedule on worker nodes (kennedy) because it has no tolerations for the master taint.

### Service that Can Run Anywhere

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: monitoring-agent
  namespace: default
spec:
  replicas: 2
  selector:
    matchLabels:
      app: monitoring-agent
  template:
    metadata:
      labels:
        app: monitoring-agent
    spec:
      tolerations:
      - key: node-role.kubernetes.io/control-plane
        operator: Exists
        effect: NoSchedule
      containers:
      - name: agent
        image: busybox
        command: ['sh', '-c', 'while true; do echo "monitoring..."; sleep 30; done']
```

**Result:** This can schedule on both master and worker nodes.

## 3. Schedule Specifically on Master

### Force Deployment to Master Only

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: master-only-service
  namespace: default
spec:
  replicas: 1
  selector:
    matchLabels:
      app: master-only-service
  template:
    metadata:
      labels:
        app: master-only-service
    spec:
      # Force to master node only
      nodeSelector:
        kubernetes.io/hostname: master
      # Allow scheduling on master despite taint
      tolerations:
      - key: node-role.kubernetes.io/control-plane
        operator: Exists
        effect: NoSchedule
      containers:
      - name: redis
        image: redis:alpine
        ports:
        - containerPort: 6379
```

**Result:** This will ONLY run on the master node.

### Alternative: Using Node Affinity (Required)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: master-required-service
  namespace: default
spec:
  replicas: 1
  selector:
    matchLabels:
      app: master-required-service
  template:
    metadata:
      labels:
        app: master-required-service
    spec:
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: kubernetes.io/hostname
                operator: In
                values:
                - master
      tolerations:
      - key: node-role.kubernetes.io/control-plane
        operator: Exists
        effect: NoSchedule
      containers:
      - name: database
        image: postgres:alpine
        env:
        - name: POSTGRES_PASSWORD
          value: "password123"
```

**Result:** Uses affinity instead of nodeSelector (more flexible for multi-master setups).

## 4. Custom Taint and Scheduling

### Create Custom Taint

```bash
# Add custom taint to a node (e.g., dedicated for databases)
kubectl taint nodes kennedy dedicated=database:NoSchedule

# Add another custom taint for high-memory workloads
kubectl taint nodes kennedy workload-type=memory-intensive:NoSchedule
```

### Verify Custom Taints

```bash
# Check kennedy node taints
kubectl describe node kennedy | grep -i taint

# Expected output might show:
# Taints: dedicated=database:NoSchedule
#         workload-type=memory-intensive:NoSchedule
```

### Schedule on Custom Tainted Node (Exact Match)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: database-service
  namespace: default
spec:
  replicas: 1
  selector:
    matchLabels:
      app: database-service
  template:
    metadata:
      labels:
        app: database-service
    spec:
      # Must tolerate the custom taint
      tolerations:
      - key: dedicated
        operator: Equal
        value: "database"
        effect: NoSchedule
      - key: workload-type
        operator: Equal
        value: "memory-intensive"
        effect: NoSchedule
      containers:
      - name: mysql
        image: mysql:8.0
        env:
        - name: MYSQL_ROOT_PASSWORD
          value: "rootpassword"
        resources:
          requests:
            memory: "2Gi"
          limits:
            memory: "4Gi"
```

### Schedule on Custom Tainted Node (Flexible)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: flexible-database
  namespace: default
spec:
  replicas: 1
  selector:
    matchLabels:
      app: flexible-database
  template:
    metadata:
      labels:
        app: flexible-database
    spec:
      # Flexible tolerations (works with any value for these keys)
      tolerations:
      - key: dedicated
        operator: Exists
        effect: NoSchedule
      - key: workload-type
        operator: Exists
        effect: NoSchedule
      containers:
      - name: mongodb
        image: mongo:latest
```

### Prefer Custom Tainted Node (Soft Scheduling)

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: preferred-database
  namespace: default
spec:
  replicas: 2
  selector:
    matchLabels:
      app: preferred-database
  template:
    metadata:
      labels:
        app: preferred-database
    spec:
      # Prefer nodes with database taint, but can run elsewhere
      affinity:
        nodeAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            preference:
              matchExpressions:
              - key: dedicated
                operator: In
                values:
                - database
          - weight: 80
            preference:
              matchExpressions:
              - key: workload-type
                operator: In
                values:
                - memory-intensive
      # Allow scheduling on tainted nodes
      tolerations:
      - key: dedicated
        operator: Exists
        effect: NoSchedule
      - key: workload-type
        operator: Exists
        effect: NoSchedule
      containers:
      - name: app
        image: nginx:alpine
```

**Result:** Will prefer the tainted node but can run on other nodes if needed.

## 5. Useful Commands

### Check Node Information

```bash
# View all nodes with labels and taints
kubectl get nodes -o wide

# Detailed node information
kubectl describe node master
kubectl describe node kennedy

# See where pods are scheduled
kubectl get pods -A -o wide
```

### Manage Taints

```bash
# Add taint
kubectl taint nodes <node-name> <key>=<value>:<effect>

# Remove taint (note the minus sign at the end)
kubectl taint nodes <node-name> <key>:<effect>-

# Examples:
kubectl taint nodes kennedy dedicated=database:NoSchedule
kubectl taint nodes kennedy dedicated:NoSchedule-
```

### Test Scheduling

```bash
# Apply a deployment
kubectl apply -f deployment.yaml

# Check where it was scheduled
kubectl get pods -o wide

# Describe pod to see tolerations
kubectl describe pod <pod-name>
```

## 6. Quick Reference

### Taint Effects

- **NoSchedule**: Don't schedule new pods, existing pods stay
- **PreferNoSchedule**: Try not to schedule, but allow if needed
- **NoExecute**: Don't schedule new pods, evict existing pods

### Toleration Operators

- **Equal**: Must match exact key=value
- **Exists**: Match key regardless of value

### Node Selection Methods

- **nodeSelector**: Simple key-value matching
- **nodeAffinity (required)**: Must match, more flexible than nodeSelector
- **nodeAffinity (preferred)**: Try to match, but allow elsewhere

## 7. Example Workflow

```bash
# 1. Apply master taint
kubectl taint nodes master node-role.kubernetes.io/control-plane:NoSchedule

# 2. Apply custom taints
kubectl taint nodes kennedy dedicated=database:NoSchedule

# 3. Deploy applications with appropriate tolerations
kubectl apply -f web-app.yaml           # Goes to any available node
kubectl apply -f master-service.yaml    # Goes to master only
kubectl apply -f database-service.yaml  # Goes to kennedy only

# 4. Verify placement
kubectl get pods -A -o wide
```

This setup gives you complete control over pod placement while following Kubernetes best practices!