package eu.pursuit.core;


public class Publication{
	private final ItemName itemName;
	private final byte [] data;		
	
	public Publication(ItemName name, byte[] data) {
		Util.checkNull(name);
		Util.checkNull(data);
		
		this.itemName = name;
		this.data = data;
	}			

	public byte[] getData() {
		return this.data;
	}

	public ItemName getItemName() {
		return itemName;
	}		
}
