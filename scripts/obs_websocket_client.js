/**
 * MotionFX OBS-WebSocket v5 Client (Node.js / Browser)
 * ====================================================
 * Demonstrates controlling MotionFX Scene Item transitions via OBS-WebSocket v5.
 * 
 * Works with native WebSocket (Node.js 22+ or modern browsers) or 'ws' package.
 */

const crypto = require('crypto');

class MotionFXClient {
    constructor(host = 'localhost', port = 4455, password = '') {
        this.url = `ws://${host}:${port}`;
        this.password = password;
        this.ws = null;
        this.pendingRequests = new Map();
    }

    async connect() {
        const WebSocketImpl = typeof WebSocket !== 'undefined' ? WebSocket : require('ws');
        return new Promise((resolve, reject) => {
            this.ws = new WebSocketImpl(this.url);

            this.ws.on('open', () => console.log(`[MotionFX-WS] Connected to ${this.url}`));
            this.ws.on('error', (err) => reject(err));

            this.ws.on('message', (raw) => {
                const msg = JSON.parse(raw);

                // 1. Hello (op 0)
                if (msg.op === 0) {
                    this.handleHello(msg.d).catch(reject);
                }
                // 2. Identified (op 2)
                else if (msg.op === 2) {
                    console.log('[MotionFX-WS] Successfully Authenticated!');
                    resolve(true);
                }
                // 3. Response (op 7)
                else if (msg.op === 7) {
                    const reqId = msg.d?.requestId;
                    if (this.pendingRequests.has(reqId)) {
                        const { resolveReq, rejectReq } = this.pendingRequests.get(reqId);
                        this.pendingRequests.delete(reqId);

                        if (msg.d?.requestStatus?.result) {
                            resolveReq(msg.d.responseData);
                        } else {
                            rejectReq(new Error(`OBS Error ${msg.d?.requestStatus?.code}: ${msg.d?.requestStatus?.comment}`));
                        }
                    }
                }
            });
        });
    }

    async handleHello(helloData) {
        const identify = {
            op: 1,
            d: {
                rpcVersion: 1,
                eventSubscriptions: 33
            }
        };

        if (helloData.authentication && this.password) {
            const { salt, challenge } = helloData.authentication;
            const secretHash = crypto.createHash('sha256').update(this.password + salt).digest('base64');
            const authResponse = crypto.createHash('sha256').update(secretHash + challenge).digest('base64');
            identify.d.authentication = authResponse;
        }

        this.ws.send(JSON.stringify(identify));
    }

    async call(requestType, requestData = {}) {
        const requestId = 'req_' + Math.random().toString(36).substr(2, 9);
        return new Promise((resolveReq, rejectReq) => {
            this.pendingRequests.set(requestId, { resolveReq, rejectReq });
            this.ws.send(JSON.stringify({
                op: 6,
                d: { requestType, requestId, requestData }
            }));
        });
    }

    async getItemId(sceneName, sourceName) {
        const res = await this.call('GetSceneItemId', { sceneName, sourceName });
        return res.sceneItemId;
    }

    async showItem(sceneName, sourceNameOrId) {
        const sceneItemId = typeof sourceNameOrId === 'string' 
            ? await this.getItemId(sceneName, sourceNameOrId) 
            : sourceNameOrId;

        console.log(`[MotionFX] Playing SHOW transition on item ${sceneItemId} in '${sceneName}'`);
        return this.call('SetSceneItemEnabled', {
            sceneName,
            sceneItemId,
            sceneItemEnabled: true
        });
    }

    async hideItem(sceneName, sourceNameOrId) {
        const sceneItemId = typeof sourceNameOrId === 'string' 
            ? await this.getItemId(sceneName, sourceNameOrId) 
            : sourceNameOrId;

        console.log(`[MotionFX] Playing HIDE (reverse) transition on item ${sceneItemId} in '${sceneName}'`);
        return this.call('SetSceneItemEnabled', {
            sceneName,
            sceneItemId,
            sceneItemEnabled: false
        });
    }

    async toggleItem(sceneName, sourceNameOrId) {
        const sceneItemId = typeof sourceNameOrId === 'string' 
            ? await this.getItemId(sceneName, sourceNameOrId) 
            : sourceNameOrId;

        const status = await this.call('GetSceneItemEnabled', { sceneName, sceneItemId });
        if (status.sceneItemEnabled) {
            return this.hideItem(sceneName, sceneItemId);
        } else {
            return this.showItem(sceneName, sceneItemId);
        }
    }

    close() {
        if (this.ws) this.ws.close();
    }
}

// Module export & CLI usage
module.exports = MotionFXClient;

if (require.main === module) {
    const args = process.argv.slice(2);
    if (args.length < 2) {
        console.log("Usage: node obs_websocket_client.js <SceneName> <SourceName> [show|hide|toggle] [password]");
        process.exit(0);
    }

    const [scene, source, action = 'toggle', password = ''] = args;
    const client = new MotionFXClient('localhost', 4455, password);

    (async () => {
        try {
            await client.connect();
            if (action === 'show') await client.showItem(scene, source);
            else if (action === 'hide') await client.hideItem(scene, source);
            else await client.toggleItem(scene, source);
        } catch (e) {
            console.error('[Error]', e.message);
        } finally {
            setTimeout(() => client.close(), 500);
        }
    })();
}
