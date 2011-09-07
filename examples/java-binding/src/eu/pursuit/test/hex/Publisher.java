package eu.pursuit.test.hex;

import java.nio.ByteBuffer;
import java.util.Arrays;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.Event.EventType;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;

public class Publisher {
	public static void main(String[] args) throws DecoderException {
		String sharedObjPath = "/home/tsilochr/Documents/tools/blackadder-git-fork/examples/java-binding/jni/eu_pursuit_client_BlackadderWrapper.o";		
		BlackadderWrapper.configureObjectFile(sharedObjPath);
		
		Strategy strategy = Strategy.NODE;
		
		String rootStr = "0000000000000000";
		ByteIdentifier root = new ByteIdentifier(Hex.decodeHex(rootStr.toCharArray()));
		
		String subscopeStr = "000000000000000a";
		ByteIdentifier subscope = new ByteIdentifier(Hex.decodeHex(subscopeStr.toCharArray()));
		
		String ridStr = "00013000f000000a";
		ByteIdentifier rid = new ByteIdentifier(Hex.decodeHex(ridStr.toCharArray()));
		
		BlackAdderClient client = BlackAdderClient.getInstance();
		
		
		ScopeID scope = new ScopeID();
		scope.addSegment(root);
		System.out.println("publish root scope");
		client.publishRootScope(root, strategy);
		
		ScopeID scope2 = new ScopeID();
		scope2.addSegment(subscope);
		System.out.println("publish subscope");
		client.publishScope(subscope, scope, strategy);
		
		scope.addSegment(subscope);
		ItemName name = new ItemName(scope, rid);
		System.out.println("publish item");
		client.publishItem(name, strategy, null);
		byte[] data = new byte[500];
		Arrays.fill(data, (byte)9);
		ByteBuffer buffer = ByteBuffer.allocateDirect(data.length);
		buffer.put(data);
		buffer.flip();
		
		while(true){
			Event event = client.getNextEvent();
			System.out.println("an event arrived");
			if(event.getType()==EventType.START_PUBLISH){
				System.out.println("Event type "+event.getType().toString());	
				System.out.println("publishing data");
				client.publishData(event.getId(), data, strategy, null);
			}
			event.freeNativeBuffer();
		}
	
	}

}
