from typing import List, Optional
from pydantic import BaseModel

class ContactTarget(BaseModel):
    contact_id: int
    name: str
    phone_number: str

class SendInvitesCommand(BaseModel):
    type: str = "SEND_INVITES"
    event_id: int
    user_id: int
    message_text: str
    contacts: List[ContactTarget]

class BridgeEvent(BaseModel):
    type: str
    event_id: Optional[int] = None
    contact_id: Optional[int] = None
    wa_message_id: Optional[str] = None
    reaction: Optional[str] = None
    info: Optional[str] = None