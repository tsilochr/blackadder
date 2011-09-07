package eu.pursuit.core;

import java.nio.ByteBuffer;

import org.apache.commons.codec.binary.Hex;

public class ItemName {
	private final ScopeID scopeId;
	private final ByteIdentifier rendezvousId;
	
	public ItemName(ScopeID scopeId, ByteIdentifier rendezvousId) {
		super();
		this.scopeId = scopeId;
		this.rendezvousId = rendezvousId;
	}

	public ScopeID getScopeId() {
		return scopeId;
	}

	public ByteIdentifier getRendezvousId() {
		return rendezvousId;
	}
	
	@Override
	public String toString(){
		return scopeId.toString()+Hex.encodeHexString(rendezvousId.getId());
	}

	public byte[] toByteArray() {
		int length = 0;
		if(scopeId != null){
			length+=scopeId.getLength();
		}
		
		if(rendezvousId!=null){
			length += rendezvousId.getId().length;			
		}
		
		ByteBuffer buffer = ByteBuffer.allocate(length);
		if(scopeId != null){
			scopeId.fill(buffer);
		}
		
		if(rendezvousId!=null){
			buffer.put(rendezvousId.getId());			
		}
		return buffer.array();
	}
}
