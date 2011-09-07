package eu.pursuit.core;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.codec.binary.Hex;

public class ScopeID {

	public static int SEGMENT_SIZE = 8;

	private final List<ByteIdentifier> list;	
	
	public ScopeID() {
		list = new ArrayList<ByteIdentifier>();		
	}
	
	public ScopeID(List<ByteIdentifier> ids) {
		Util.checkNull(ids);
		list = new ArrayList<ByteIdentifier>(ids);	
		
		StringBuilder strBuilder = new StringBuilder();
		for (ByteIdentifier byteIdentifier : ids) {
			strBuilder.append(Hex.encodeHex(byteIdentifier.getId()));
		}		
	}	
	
	public ScopeID(ByteIdentifier id) {
		this();
		this.list.add(id);
	}

	public ScopeID addSegment(ByteIdentifier identifier) {
		Util.checkNull(identifier);	
		if(identifier.length() != SEGMENT_SIZE){
			throw new IllegalArgumentException("invalid segment size: "+identifier.length());
		}		
		
		if(!this.list.add(identifier)){
			throw new RuntimeException("failed to add to list");
		}			
		return this;
	}
	
	public int getLength() {
		int totalLength = 0;
		for (ByteIdentifier id : list) {
			totalLength += id.getId().length;
		}
		return totalLength;
	}
	
	public byte [] toByteArray(){				
		ByteBuffer buffer = ByteBuffer.allocate(getLength());
		fill(buffer);		
		return buffer.array();
	}
	
	public void fill(ByteBuffer buffer) {
		for (ByteIdentifier id : list) {
			buffer.put(id.getId());
		}
	}		

	@Override
	public int hashCode() {
		return list.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		ScopeID otherScope = (ScopeID)obj;
		return list.equals(otherScope.list);
	}			
}
