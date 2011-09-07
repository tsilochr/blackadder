/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package eu.pursuit.chat;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.Strategy;
import java.util.Scanner;
import org.apache.commons.codec.binary.StringUtils;

/**
 *
 * @author summer
 */
class Sender implements Runnable{
    private final BlackAdderClient blackadder;
    private final byte[] id;

    public Sender(BlackAdderClient blackadder, ItemName name) {
        this.blackadder = blackadder;
        this.id = name.toByteArray();
    }

    public void run() {
        System.out.println("Starting publisher thread");
        Scanner scanner;
        while(true){
            scanner = new Scanner(System.in);
            String text = scanner.nextLine().trim();
            byte [] asciiData = StringUtils.getBytesUsAscii(text);
            System.err.println("[publishig text]");
            blackadder.publishData(id, asciiData, Strategy.DOMAIN_LOCAL);
        }
    }
    
}
