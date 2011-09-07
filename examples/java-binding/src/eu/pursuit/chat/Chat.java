/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package eu.pursuit.chat;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;
import java.util.Arrays;
import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;
import org.apache.commons.codec.binary.StringUtils;

public class Chat {

    public static void main(String[] args) throws DecoderException, InterruptedException {
        String objectFile = "/home/summer/blackadder-java-netbeans/jni/eu_pursuit_client_BlackadderWrapper.o";
        BlackadderWrapper.configureObjectFile(objectFile);
        BlackAdderClient blackadder = BlackAdderClient.getInstance();
        Strategy DOMAIN_LOC = Strategy.DOMAIN_LOCAL;

        /*IDs*/
        String rootScopeStr = "0000000000000002";
        ByteIdentifier rootId = new ByteIdentifier(Hex.decodeHex(rootScopeStr.toCharArray()));

        String XenofonStr = "0000000000000001";
        ByteIdentifier xenRid = new ByteIdentifier(Hex.decodeHex(XenofonStr.toCharArray()));

        String ChristosStr = "0000000000000002";
        ByteIdentifier christosRid = new ByteIdentifier(Hex.decodeHex(ChristosStr.toCharArray()));

        String MartinStr = "0000000000000003";
        ByteIdentifier martinRid = new ByteIdentifier(Hex.decodeHex(MartinStr.toCharArray()));

        String MaysStr = "0000000000000004";
        ByteIdentifier mayRid = new ByteIdentifier(Hex.decodeHex(MaysStr.toCharArray()));

        blackadder.publishRootScope(rootId, DOMAIN_LOC);

        ScopeID rootScope = new ScopeID(rootId);
        ItemName xenName = new ItemName(rootScope, xenRid);
        ItemName christosName = new ItemName(rootScope, christosRid);
        ItemName martinName = new ItemName(rootScope, martinRid);
        ItemName MayName = new ItemName(rootScope, mayRid);

        System.out.println("subscribing to Xenofon " + Hex.encodeHexString(xenName.toByteArray()));
        blackadder.subscribeItem(xenName, DOMAIN_LOC);
        Thread.sleep(300);

        System.out.println("subscribing to Martin " + Hex.encodeHexString(martinName.toByteArray()));
        blackadder.subscribeItem(martinName, DOMAIN_LOC);
        Thread.sleep(300);

        System.out.println("subscribing to May " + Hex.encodeHexString(MayName.toByteArray()));
        blackadder.subscribeItem(MayName, DOMAIN_LOC);
        Thread.sleep(300);

        Thread senderThread = new Thread(new Sender(blackadder, christosName));
        senderThread.start();

        while (true) {
            System.err.print("[waiting for next event...");            
            Event event = blackadder.getNextEvent();           
            System.err.println("got it]");            
            System.err.println(event.getType());            
            if (event.getType().equals(Event.EventType.PUBLISHED_DATA)) {
                System.err.println("got a packet from network, id " + Hex.encodeHexString(event.getId()));
                if (Arrays.equals(event.getId(), martinName.toByteArray())) {
                    System.out.print("[Martin ");
                } else if (Arrays.equals(event.getId(), MayName.toByteArray())) {
                    System.out.print("[May ");
                } else if (Arrays.equals(event.getId(), xenName.toByteArray())) {
                    System.out.print("[Xenofon ");
                } else {
                    System.out.print("[Unknown guy ");
                }
            }
            System.out.print("says]: ");
            String text = StringUtils.newStringUsAscii(event.getDataCopy());
            System.out.println(text);
            
            /*don't forget to free the event buffer*/
            event.freeNativeBuffer();
        }
    }
}
