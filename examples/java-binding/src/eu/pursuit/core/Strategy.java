package eu.pursuit.core;

public enum Strategy {
	NODE((byte) 0), 
	LINK_LOCAL((byte) 1), 
	DOMAIN_LOCAL((byte) 2);
	
	private final byte byteValue;
	
	private Strategy(byte val){
		this.byteValue = val;
	}
	
	public byte byteValue(){
		return byteValue;
	}

	public static Strategy byValue(byte b) {
		Strategy strategy = null;
		for (Strategy strat: values()) {
			if(strat.byteValue == b){
				strategy = strat;
				break;
			}
		}
		
		return strategy;
	}

}
