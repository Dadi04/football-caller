import sys
import os

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from app.cpp_client import CppBridgeClient
from app.schemas import BridgeEvent

def main():
    client = CppBridgeClient()
    print("Python ZeroMQ bridge started and is waiting for command from C++")

    try:
        while True:
            cmd = client.receive_command()
            cmd_type = cmd.get("type")
            print(f"Received command from C++: {cmd}")

            if cmd_type == "PING":
                reply = BridgeEvent(
                    type="PONG",
                    info="Connection works"
                )
                client.send_event(reply)
            elif cmd_type == "SEND_INVITES":
                event_id = cmd.get("event_id")
                contacts = cmd.get("contacts", [])
                print(f"Simulating sending messages for {len(contacts)} contacts")
                for c in contacts:
                    fake_msg_id = f"wa_msg_{event_id}_{c["contact_id"]}"
                    client.send_event(BridgeEvent(
                        type="MESSAGE_SENT",
                        event_id=event_id,
                        contact_id=c["contact_id"],
                        wa_message_id=fake_msg_id
                    ))
    except KeyboardInterrupt:
        print("\nPython bridge shutting down")
    finally:
        client.close()

if __name__ == "__main__":
    main()