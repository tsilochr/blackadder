package eu.pursuit.core;



public class ForwardIdentifier{
	private final ByteIdentifier linkID;

	public ForwardIdentifier(ByteIdentifier id) {
		Util.checkNull(id);
		this.linkID = id;
	}

	public ByteIdentifier getLinkID() {
		return linkID;
	}

	public byte[] toByteArray() {
		return linkID.getId();
	}	
}
