package eu.pursuit.core;


public class Util {

	/*
	 * Throw a NullpointerException if o is null 
	 */
	public static void checkNull(Object o){
		if (o==null)
			throw new NullPointerException();
	}
	
	public static void checkNull(Object o, String mesg) {
		if (o==null)
			throw new NullPointerException(mesg);		
	}

	public static void checkIllegal(Object o, String fieldName) {
		if(o == null){
			throw new IllegalArgumentException("field "+fieldName+" not initialized");
		}
		
	}
	
	public static <T> void printArray(T [] array){
		for (T t : array) {
			System.out.print(t+" ");
		}
		System.out.println();
	}
	
	public static void printArray(byte [] array){
		for (byte b : array) {
			System.out.print(b+" ");
		}
		System.out.println();
	}		
}
