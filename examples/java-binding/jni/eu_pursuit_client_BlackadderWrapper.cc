#include "eu_pursuit_client_BlackadderWrapper.h"
#include "blackadder.hpp"
#include "bitvector.hpp"
#include <string>
#include <stdio.h>

using std::string;

void print_contents(char *data_ptr, int length){
	for(int i=0; i<length; i++){
		printf("%d ", data_ptr[i]);	
	}
	printf("\n");
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    create_new_ba
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_eu_pursuit_client_BlackadderWrapper_create_1new_1ba
  (JNIEnv *, jobject, jint userspace){
	bool user = userspace? true : false;
	Blackadder *ba_ptr = new Blackadder(user);
	return (jlong) ba_ptr;
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    delete_ba
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_delete_1ba
  (JNIEnv *, jobject, jlong ba_ptr){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;
	delete ba;
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1scope
  (JNIEnv *env, jobject obj, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray fid){

	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);	

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	/*
	 * the call to blackadder 
	 */
	ba->publish_scope((char *)scope_ptr, scope_length, (char *)scope_prefix_ptr, scope_prefix_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);	

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	/*
	 * the call to blackadder 
	 */
	ba->publish_info((char *)rid_ptr, rid_length, (char *)sid_ptr, sid_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unpublish_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unpublish_1scope
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}
	
	/*
	 * the call to blackadder 
	 */
	ba->unpublish_scope((char *)scope_ptr, scope_length, (char *)scope_prefix_ptr, scope_prefix_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unpublish_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unpublish_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);
	string id = rid_length == 0? "" : std::string((char *)rid_ptr, rid_length);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);
	string prefix = sid_length == 0? "" : std::string((char *)sid_ptr, sid_length);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	/*
	 * the call to blackadder 
	 */
	ba->unpublish_info((char *)rid_ptr, rid_length, (char *)sid_ptr, sid_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_subscribe_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1subscribe_1scope
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}
	
	/*
	 * the call to blackadder 
	 */
	ba->subscribe_scope((char *)scope_ptr, scope_length, (char *)scope_prefix_ptr, scope_prefix_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_subscribe_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1subscribe_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	/*
	 * the call to blackadder 
	 */
	ba->subscribe_info((char *)rid_ptr, rid_length, (char *)sid_ptr, sid_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unsubscribe_scope
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unsubscribe_1scope
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray scope, jbyteArray scope_prefix, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * scope_ptr = (*env).GetByteArrayElements(scope, &copy);
	int scope_length = (*env).GetArrayLength(scope);

	jbyte *scope_prefix_ptr = (*env).GetByteArrayElements(scope_prefix, &copy);
	int scope_prefix_length = (*env).GetArrayLength(scope_prefix);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}
	
	/*
	 * the call to blackadder 
	 */
	ba->unsubscribe_scope((char *)scope_ptr, scope_length, (char *)scope_prefix_ptr, scope_prefix_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(scope, scope_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(scope_prefix, scope_prefix_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_unsubscribe_item
 * Signature: (J[B[BB[B)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1unsubscribe_1item
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray rid, jbyteArray sid, jbyte strategy, jbyteArray fid){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte * rid_ptr = (*env).GetByteArrayElements(rid, &copy);
	int rid_length = (*env).GetArrayLength(rid);
	string id = rid_length == 0? "" : std::string((char *)rid_ptr, rid_length);

	jbyte *sid_ptr = (*env).GetByteArrayElements(sid, &copy);
	int sid_length = (*env).GetArrayLength(sid);
	string prefix = sid_length == 0? "" : std::string((char *)sid_ptr, sid_length);

	char *lid;
	lid = 0;

	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	/*
	 * the call to blackadder 
	 */
	ba->unsubscribe_info((char *)rid_ptr, rid_length, (char *)sid_ptr, sid_length, (char)strategy, lid);

	(*env).ReleaseByteArrayElements(rid, rid_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(sid, sid_ptr, (jint)0);	
	
	if(lid != 0){	
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);		
	}
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_data_direct
 * Signature: (J[BB[BLjava/nio/ByteBuffer;I)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1data_1direct
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray name, jbyte strategy, jbyteArray fid, jobject jbytebuffer, jint length){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;


	jboolean copy = (jboolean)false;
	jbyte *name_ptr = (*env).GetByteArrayElements(name, &copy);
	int name_length = (*env).GetArrayLength(name);
	
	char* lid;
	lid = 0;
	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	char *data_ptr = (char *)(*env).GetDirectBufferAddress(jbytebuffer);	
	
	ba->publish_data((char *)name_ptr, name_length, (char)strategy, lid, (char *)data_ptr, (int)length);
	
	(*env).ReleaseByteArrayElements(name, name_ptr, (jint)0);
	if(lid!=0){
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);
	}

}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_publish_data
 * Signature: (J[BB[B[BI)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1publish_1data
  (JNIEnv *env, jobject, jlong ba_ptr, jbyteArray name, jbyte strategy, jbyteArray fid, jbyteArray data, jint datalen){
	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;

	jboolean copy = (jboolean)false;

	jbyte *name_ptr = (*env).GetByteArrayElements(name, &copy);
	int name_length = (*env).GetArrayLength(name);
	
	char* lid;
	lid = 0;
	if(fid != NULL && (*env).GetArrayLength(fid) > 0) {
		lid = (char *) (*env).GetByteArrayElements(fid, &copy);
	}

	jbyte *data_ptr = (*env).GetByteArrayElements(data, &copy);
//	print_contents((char *)data_ptr, (int)datalen);	

	ba->publish_data((char *)name_ptr, name_length, (char)strategy, lid, (char *)data_ptr, (int)datalen);
	

	(*env).ReleaseByteArrayElements(name, name_ptr, (jint)0);
	(*env).ReleaseByteArrayElements(data, data_ptr, (jint)0);
	if(lid!=0){
		(*env).ReleaseByteArrayElements(fid, (jbyte *)lid, (jint)0);
	}	
}


/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_nextEvent_direct
 * Signature: (JLeu/pursuit/client/EventInternal;)J
 */
JNIEXPORT jlong JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1nextEvent_1direct
  (JNIEnv *env, jobject, jlong ba_ptr, jobject obj_EventInternal){

	Blackadder *ba;
	ba = (Blackadder *)ba_ptr;
	Event *ev = ba->getNewEvent();

	static jclass cls = (*env).GetObjectClass(obj_EventInternal);
	
	static jfieldID evTypeField = (*env).GetFieldID(cls, "type", "B");
	if (evTypeField == NULL) {
		printf("JNI: error getting field id for Event.type\n");
		return 0;
	}

	static	jfieldID evIdField = (*env).GetFieldID(cls, "id", "[B");
	if (evTypeField == NULL) {
		printf("JNI: error getting field id for Event.id\n");
		return 0;
	}

	static	jfieldID evDataField = (*env).GetFieldID(cls, "data", "Ljava/nio/ByteBuffer;");
	if (evDataField == NULL) {
		printf("JNI: error getting field id for Event.data\n");
		return 0;
	}	

	//copy type byte
	(*env).SetByteField(obj_EventInternal, evTypeField, (jbyte) ev->type);

	//copy id array
	const char *id_ptr = ev->id.c_str();	
	jbyteArray idArray = (*env).NewByteArray(ev->id.length());
	(*env).SetByteArrayRegion(idArray, 0, ev->id.length(), (jbyte *) id_ptr);
	(*env).SetObjectField(obj_EventInternal, evIdField, idArray);

	jobject jbuff = (*env).NewDirectByteBuffer((void*)ev->data, (jlong) ev->data_len); 
	(*env).SetObjectField(obj_EventInternal, evDataField, jbuff);
	
	return (jlong)ev;
}

/*
 * Class:     eu_pursuit_client_BlackadderWrapper
 * Method:    c_delete_event
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_eu_pursuit_client_BlackadderWrapper_c_1delete_1event
  (JNIEnv *, jobject, jlong ba_ptr, jlong ev_ptr){
	Event *event = (Event *)ev_ptr;
	free(event->buffer);
	delete event;
}


