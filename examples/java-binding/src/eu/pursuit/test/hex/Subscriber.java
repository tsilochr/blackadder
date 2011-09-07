package eu.pursuit.test.hex;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.Event.EventType;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;

public class Subscriber {
	public static void main(String[] args) throws DecoderException {
		String sharedObjPath = "/media/WD Passport/source_code/blackadder/v0.2beta/java-binding/jni/eu_pursuit_client_BlackadderWrapper.o";
		BlackadderWrapper.configure(sharedObjPath);

		Strategy strategy = Strategy.NODE;

		String rootStr = "0000000000000000";
		ByteIdentifier root = new ByteIdentifier(Hex.decodeHex(rootStr
				.toCharArray()));

		String subscopeStr = "000000000000000a";
		ByteIdentifier subscope = new ByteIdentifier(Hex.decodeHex(subscopeStr
				.toCharArray()));

		String ridStr = "00013000f000000a";
		ByteIdentifier rid = new ByteIdentifier(Hex.decodeHex(ridStr
				.toCharArray()));

		BlackAdderClient client = new BlackAdderClient();

		ScopeID rootScope = new ScopeID();
		rootScope.addSegment(root);
//		System.out.println("subscribe root scope");
//		client.subscribeScope(rootScope, null, strategy, null);

		ScopeID nestedScope = new ScopeID();
		nestedScope.addSegment(subscope);

		System.out.println("subscribe subscope");
		client.subscribeScope(nestedScope, rootScope, strategy, null);

		while(true){
			Event event = client.getNextEvent();
			System.out.println("an event arrived");
			System.out.println("Event type " + event.getType().toString());
			String str = new String(Hex.encodeHex(event.getId()));
			System.out.println("name is " + str);
			if(event.getType() == EventType.PUBLISHED_DATA){
				byte[] data = event.getDataCopy();
				printbytes(data);
			}
		}		
	}
	
	private static void printbytes(byte[] data) {
		for (byte b : data) {
			System.out.print(b+" ");
		}
		System.out.println();
		
	}

}
