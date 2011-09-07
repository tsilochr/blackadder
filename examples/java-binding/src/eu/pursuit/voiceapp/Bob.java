package eu.pursuit.voiceapp;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Random;
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

public class Bob {
	private static String objectFilePath = "";
	private static Strategy strategy = Strategy.NODE;

	public static void main(String[] args) {
		readOptions(args);
		BlackadderWrapper.configureObjectFile(objectFilePath);
		BlackAdderClient blackadder = BlackAdderClient.getInstance();
		System.out.print("Enter my name: ");
		Scanner scaner = new Scanner(System.in);
		String myNameStr = scaner.nextLine().trim();
		
                System.out.println("my name is "+myNameStr); 
		
                ByteIdentifier voiceappPref = new ByteIdentifier(Configuration.VOICE_APP_PREFIX.getBytes(), 8);		
		blackadder.publishRootScope(voiceappPref, strategy );
		
		ScopeID voiceScope = new ScopeID(voiceappPref);
		ByteIdentifier BobAgentRid = new ByteIdentifier(myNameStr.getBytes(), 8);
		ItemName agentName = new ItemName(voiceScope, BobAgentRid);
                System.out.println("publishing agent under to "+Hex.encodeHexString(agentName.toByteArray()));
		blackadder.publishItem(agentName, strategy);
		
		Event event = blackadder.getNextEvent();
		
		//compute two new rids
		Random random =  new Random(System.currentTimeMillis());
		byte [] AliceToBobArray = new byte[8];
		random.nextBytes(AliceToBobArray);
		ItemName AliceToBobChannelName = new ItemName(voiceScope, new ByteIdentifier(AliceToBobArray));
                System.out.println("Alice to Bob channel "+Hex.encodeHexString(AliceToBobChannelName.toByteArray()));
		blackadder.subscribeItem(AliceToBobChannelName, strategy);
		
		byte [] BobToAliceArray = new byte[8];
		random.nextBytes(BobToAliceArray);
		ItemName BobToAliceChannelName = new ItemName(voiceScope, new ByteIdentifier(BobToAliceArray));
                System.out.println("Bob to Alice channel "+Hex.encodeHexString(BobToAliceChannelName.toByteArray()));
		blackadder.publishItem(BobToAliceChannelName, strategy);
		
		ByteBuffer buffer = ByteBuffer.allocateDirect(16);		
		buffer.put(AliceToBobArray);
		buffer.put(BobToAliceArray);
		
		blackadder.publishData(event.getId(), buffer, strategy);
		event.freeNativeBuffer();
		
		//start sending thread			
		Thread sendingThread = new Thread(new SenderThread(blackadder, BobToAliceChannelName));
		sendingThread.start();
		
		//start player thread		
		BlockingQueue<byte []> queue = new LinkedBlockingQueue<byte[]>();
		Thread soundPlayerThread = new  Thread(new JavaSoundPlayer(queue));
		soundPlayerThread.start();
		
		AliceToBobArray = AliceToBobChannelName.toByteArray();
                
		while(true){
			event = blackadder.getNextEvent();
			if(event.getType() == EventType.PUBLISHED_DATA && Arrays.equals(AliceToBobArray, event.getId())){
				System.out.println("Got a packet from Alice, size "+event.getDataLength());
				queue.offer(event.getDataCopy());				
			}
			event.freeNativeBuffer();
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
