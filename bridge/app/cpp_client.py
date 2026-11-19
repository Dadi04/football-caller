import zmq
from config import ZMQ_CORE_TO_BRIDGE_ADDR, ZMQ_BRIDGE_TO_CORE_ADDR
from app.schemas import BridgeEvent

class CppBridgeClient:
    def __init__(self):
        self.ctx = zmq.Context()

        self.pull_socket = self.ctx.socket(zmq.PULL)
        self.pull_socket.bind(ZMQ_CORE_TO_BRIDGE_ADDR)

        self.push_socket = self.ctx.socket(zmq.PUSH)
        self.push_socket.connect(ZMQ_BRIDGE_TO_CORE_ADDR)

    def receive_command(self) -> dict:
        return self.pull_socket.recv_json()

    def send_event(self, event: BridgeEvent) -> None:
        self.push_socket.send_json(event.model_dump(exclude_none=True))

    def close(self) -> None:
        self.pull_socket.close()
        self.push_socket.close()
        self.ctx.term()