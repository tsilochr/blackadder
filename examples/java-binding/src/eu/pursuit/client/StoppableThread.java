package eu.pursuit.client;

public class StoppableThread extends Thread {
	private volatile boolean shutDown = false;
	
	public synchronized boolean isShutDown(){
		return shutDown;
	}
	
	public synchronized void shutDown(){
		this.shutDown = true;
	}
	
	/**
	 * Sets this thread's name to a structured form: name+"/"+this class's name.
	 * @param name
	 */
	public void setNamePrefix(String name){
		String n = name;
		if(!n.endsWith("/")){
			n = n.concat("/");
		}
		n = n.concat(this.getClass().getSimpleName());		
		this.setName(n);
	}
}
