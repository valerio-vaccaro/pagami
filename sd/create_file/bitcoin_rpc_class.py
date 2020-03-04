import time, requests, json, re

class RPCHost(object):

    def __init__(self, url):
        self.session = requests.Session()
        if re.match(r'.*\.onion/*.*', url):
            self.session.proxies = {
                'http': 'socks5h://localhost:9050',
                'https': 'socks5h://localhost:9050',
            }
        self.url = url

    def call(self, rpc_method, *params):
        payload = json.dumps({"method": rpc_method, "params": list(params), "jsonrpc": "2.0"})
        connected = False
        max_tries = 5
        for tries in range(max_tries):
            try:
                response = self.session.post(self.url, headers={'content-type': 'application/json'}, data=payload)
                connected = True
                break
            except requests.exceptions.ConnectionError:
                time.sleep(10)

        if not connected:
            raise Exception('Failed to connect for remote procedure call.')

        if not response.status_code in (200, 500):
            raise Exception(f'RPC connection failure: {response.status_code} {response.reason}')

        response_json = response.json()
        if 'error' in response_json and response_json['error']:
            raise Exception('Error in RPC call: {responseJSON["error"]}')
        return response_json['result']
