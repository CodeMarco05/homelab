# Kubernetes

The cluster runs k3s — a lightweight Kubernetes distribution that works well on low-power hardware. Three nodes form the cluster: one control plane and two workers.

## Installation

### Control plane

```bash
curl -sfL https://get.k3s.io | sh -s - server \
  --disable traefik \
  --disable servicelb \
  --flannel-backend=none \
  --disable-network-policy \
  --cluster-cidr=10.42.0.0/16
```

Traefik and the built-in load balancer are disabled — the cluster uses Cilium for networking and MetalLB for load balancing instead.

### Worker nodes

Grab the node token from the control plane:

```bash
cat /var/lib/rancher/k3s/server/node-token
```

Then on each worker:

```bash
curl -sfL https://get.k3s.io | K3S_URL=https://10.0.1.10:6443 \
  K3S_TOKEN=<node-token> sh -
```

## Networking

### Cilium

Cilium handles pod networking and enforces network policies.

```bash
helm repo add cilium https://helm.cilium.io/
helm install cilium cilium/cilium \
  --namespace kube-system \
  --set kubeProxyReplacement=true \
  --set k8sServiceHost=10.0.1.10 \
  --set k8sServicePort=6443
```

### MetalLB

MetalLB assigns real IP addresses to `LoadBalancer` services from a pool on the local network.

```yaml
apiVersion: metallb.io/v1beta1
kind: IPAddressPool
metadata:
  name: homelab-pool
  namespace: metallb-system
spec:
  addresses:
    - 10.0.1.200-10.0.1.220
---
apiVersion: metallb.io/v1beta1
kind: L2Advertisement
metadata:
  name: homelab
  namespace: metallb-system
```

## Storage

Longhorn provides replicated persistent storage across the cluster nodes. It runs as a DaemonSet and handles automatic replication and snapshots.

```bash
helm install longhorn longhorn/longhorn \
  --namespace longhorn-system \
  --create-namespace \
  --set defaultSettings.defaultReplicaCount=2
```

> Longhorn exposes a UI at its `LoadBalancer` service IP. Protect it with Authentik forward auth before exposing externally.

## Monitoring

Prometheus and Grafana run in the `monitoring` namespace via the `kube-prometheus-stack` Helm chart.

```bash
helm install kube-prometheus-stack \
  prometheus-community/kube-prometheus-stack \
  --namespace monitoring \
  --create-namespace \
  --set grafana.adminPassword=changeme \
  --set prometheus.prometheusSpec.retention=30d
```

### Node resource overview

| Node | CPU Request Limit | Memory Request Limit |
|---|---|---|
| k3s-control | 500m | 1 Gi |
| k3s-worker-01 | 4000m | 12 Gi |
| k3s-worker-02 | 4000m | 12 Gi |

## Useful Commands

```bash
# Check node status
kubectl get nodes -o wide

# Watch pod events across all namespaces
kubectl get events -A --sort-by='.lastTimestamp' | tail -30

# Force restart a deployment
kubectl rollout restart deployment/<name> -n <namespace>

# Get resource usage per node
kubectl top nodes
```
