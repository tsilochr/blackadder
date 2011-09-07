package eu.pursuit.client;

import java.nio.ByteBuffer;

public class EventInternal {
	private byte type = - 1;
	byte [] id = null; 
	ByteBuffer data = null;
	
	public byte getType() {
		return type;
	}
	
	public void setType(byte type) {
		this.type = type;
	}
	
	public byte[] getId() {
		return id;
	}
	
	public void setId(byte[] id) {
		this.id = id;
	}
	
	public ByteBuffer getData() {
		return data;
	}
	
	public void setData(ByteBuffer data) {
		this.data = data;
	}	
}
