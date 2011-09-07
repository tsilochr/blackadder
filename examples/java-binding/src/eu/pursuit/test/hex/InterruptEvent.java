package eu.pursuit.test.hex;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;

public class InterruptEvent {
	public static void main(String[] args) {
		String sharedObjPath = "/media/WD Passport/source_code/blackadder/v0.2beta/java-binding/jni/eu_pursuit_client_BlackadderWrapper.o";
		BlackadderWrapper.configure(sharedObjPath);

		BlackAdderClient client = new BlackAdderClient();

		ReaderThread t = new ReaderThread(client);
		t.start();
		try {
			System.out.println("sleeping");
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		System.out.println("got up");
		if (t.isBlocked()) {
			t.interrupt();
		}
	}

	private static class ReaderThread extends Thread {
		private BlackAdderClient ba;
		private boolean blocked = true;

		public ReaderThread(BlackAdderClient cl) {
			ba = cl;
		}

		public synchronized boolean isBlocked() {
			return blocked;
		}

		private synchronized void setBlocked(boolean blocked) {
			this.blocked = blocked;
		}

		@Override
		public void run() {
			try {
				setBlocked(true);
				ba.getNextEvent();
			} catch (Exception e) {
				System.out.println("caught an exception");
				e.printStackTrace();
			}
		}

	}

}
