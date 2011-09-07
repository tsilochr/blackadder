package eu.pursuit.test;

import java.nio.ByteBuffer;
import java.util.Arrays;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.Event.EventType;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;

public class Publisher {
	public static int TIMES = 1;
	public static void main(String[] args) {
		
		String sharedObjPath = "/home/summer/blackadder-java-netbeans/jni/eu_pursuit_client_BlackadderWrapper.o";
		
		BlackadderWrapper.configure(sharedObjPath);
		
		BlackAdderClient client = new BlackAdderClient();
		ScopeID rootScope = new ScopeID();
		ByteIdentifier rootScopeId = new ByteIdentifier((byte)0, 8); 
		rootScope.addSegment(rootScopeId);
		
		client.publishRootScope(rootScopeId, Strategy.NODE);
		
		ByteIdentifier rid = new ByteIdentifier((byte)1, 8);
		ItemName name = new ItemName(rootScope, rid);
		
		System.out.println("Publishing");
		client.publishItem(name, Strategy.NODE, null);
		System.out.println("done");
		System.out.println("waiting for events");
		Event event = client.getNextEvent();
		System.out.println("got a notification");
		
		byte [] payload = new byte[1000];
		Arrays.fill(payload, (byte)5);
		ByteBuffer buffer = ByteBuffer.allocateDirect(payload.length);
		buffer.put(payload);
		buffer.flip();		
		if(event.getType() == EventType.START_PUBLISH){		
			System.out.println("got event");
			for (int i = 0; i < TIMES; i++) {
				System.out.println("apcket "+i);
				client.publishData(event.getId(), payload, Strategy.NODE, null);
			}						
		}
		client.disconnect();
	}

}
