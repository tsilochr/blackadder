package eu.pursuit.voiceapp;

import java.io.File;
import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;
import eu.pursuit.core.Event.EventType;
import org.apache.commons.codec.binary.Hex;

public class Alice {
	
	private static String objectFilePath = "";
	private static Strategy strategy = Strategy.NODE;

	public static void main(String[] args) {
		readOptions(args);
		BlackadderWrapper.configureObjectFile(objectFilePath);
		BlackAdderClient blackadder = BlackAdderClient.getInstance();
		
		System.out.print("Enter callee name: ");
		Scanner scaner = new Scanner(System.in);
		String BobAgentStr = scaner.nextLine().trim();
		
		ByteIdentifier voiceappPref = new ByteIdentifier(Configuration.VOICE_APP_PREFIX.getBytes(), 8);
		ScopeID voiceScope = new ScopeID(voiceappPref);
		ByteIdentifier BobAgentRid = new ByteIdentifier(BobAgentStr.getBytes(), 8);
		ItemName BobAgentName = new ItemName(voiceScope, BobAgentRid );
                System.out.println("subscribing to "+Hex.encodeHexString(BobAgentName.toByteArray()));
		blackadder.subscribeItem(BobAgentName , strategy);
		
		Event netEvent = blackadder.getNextEvent();
		byte [] AliceToBobArray = null;
		byte [] BobToAliceArray = null;
		if(netEvent.getType() == EventType.PUBLISHED_DATA){
			 AliceToBobArray = netEvent.getData(0, 8);
			 BobToAliceArray = netEvent.getData(8, 8);
		}
		netEvent.freeNativeBuffer();
		
		ByteIdentifier AliceToBobRid = new ByteIdentifier(AliceToBobArray);
		ItemName AliceToBobChannelName = new ItemName(voiceScope, AliceToBobRid);
                System.out.println("Alice to Bob channel: "+Hex.encodeHexString(AliceToBobChannelName.toByteArray()));
		blackadder.publishItem(AliceToBobChannelName, strategy);
		
		ByteIdentifier BobToAliceRid = new ByteIdentifier(BobToAliceArray);
		ItemName BobToAliceChannelName = new ItemName(voiceScope, BobToAliceRid);
                System.out.println("Bob to Alice channel: "+Hex.encodeHexString(BobToAliceChannelName.toByteArray()));
		blackadder.subscribeItem(BobToAliceChannelName, strategy );
		
		//start sending voice packets in a background thread		
		SenderThread sendingThread= new SenderThread(blackadder, AliceToBobChannelName);
		sendingThread.start();						
		
		BlockingQueue<byte []> queue = new LinkedBlockingQueue<byte[]>();
		Thread soundPlayerThread = new  Thread(new JavaSoundPlayer(queue));
		soundPlayerThread.start();
		
		BobToAliceArray = BobToAliceChannelName.toByteArray();
                
		while(true){
			netEvent = blackadder.getNextEvent();
			if(netEvent.getType() == EventType.PUBLISHED_DATA && Arrays.equals(netEvent.getId(), BobToAliceArray)){
				System.out.println("Got a packet from Bob, size "+netEvent.getDataLength());
				queue.offer(netEvent.getDataCopy());				
			}
			netEvent.freeNativeBuffer();
		}
                
	}

	private static void readOptions(String[] args) {
		if(args.length != 2){
			System.out.println("Invalid args length. Exiting");
			System.exit(1);
		}
		
		String option = args[0];
		if(!option.equals("-l")){
			System.out.printf("Invalid option: %s Exiting\n", option);
                        System.out.println();
			System.exit(1);
		}
		
		String o_file = args[1];
		File f = new File(o_file);
		if(!f.exists() || !f.isFile()){
			System.out.println("cannot find file "+o_file);
			System.exit(1);
		}
		
		objectFilePath  = o_file;		
	}

}
