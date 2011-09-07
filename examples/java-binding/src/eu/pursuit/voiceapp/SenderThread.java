package eu.pursuit.voiceapp;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.Strategy;

public class SenderThread extends Thread{
	private BlockingQueue<byte []> queue = new LinkedBlockingQueue<byte[]>();
	private final BlackAdderClient blackadder;
	private final byte[] name;
	public SenderThread(BlackAdderClient blackadder, ItemName sendingSessionName) {
		this.blackadder = blackadder;
		this.name = sendingSessionName.toByteArray();
	}
	@Override
	public void run(){
		Thread recorderThread = new Thread(new JavaSoundRecorder(queue));
		recorderThread.start();
		while(true){
			try {
				byte[] bytes = queue.take();				
				this.blackadder.publishData(name, bytes, Strategy.DOMAIN_LOCAL);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}
