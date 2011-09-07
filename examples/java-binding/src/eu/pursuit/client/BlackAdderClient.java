package eu.pursuit.client;

import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;

import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.ForwardIdentifier;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.Publication;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;
import eu.pursuit.core.Util;
import org.apache.commons.codec.binary.Hex;

public class BlackAdderClient{	
	
	public static final byte[] EMPTY_ARRAY = new byte[0];
	
	private final BlackadderWrapper baWrapper;
	private boolean closed;

	public BlackAdderClient(){
		this(true);
	}
	
	public BlackAdderClient(boolean userspace){			
		baWrapper = new BlackadderWrapper(userspace);
		closed = false;
	}
	
	public void publishRootScope(ByteIdentifier scope, Strategy strategy){
		publishScope(scope, null, strategy, null);
	}
	
	public void publishRootScope(ByteIdentifier scope, Strategy strategy, ForwardIdentifier fID){
		publishScope(scope, null, strategy, fID);
	}
	
	public void publishScope(ByteIdentifier scope, ScopeID prefixScope,	Strategy strategy){
		publishScope(scope, prefixScope, strategy, null);
	}

	public void publishScope(ByteIdentifier scope, ScopeID prefixScope,
			Strategy strategy, ForwardIdentifier fID) {	
		byte [] scopeStr = scope == null? EMPTY_ARRAY: scope.getId();
		byte [] prefixStr = prefixScope == null? EMPTY_ARRAY : prefixScope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.publishScope(scopeStr, prefixStr, strategyValue, fidArray);
	}
	
	public void republishScope(ScopeID scope, ScopeID superScope, Strategy strategy) {
		republishScope(scope, superScope, strategy, null);
	}
	
	public void republishScope(ScopeID scope, ScopeID superScope, Strategy strategy, ForwardIdentifier fID) {
		Util.checkNull(scope);
		Util.checkNull(superScope);
		
		byte [] scopeStr = scope == null? EMPTY_ARRAY: scope.toByteArray();
		byte [] prefixStr = superScope == null? EMPTY_ARRAY : superScope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.publishScope(scopeStr, prefixStr, strategyValue, fidArray);
	}

	public void publishItem(ItemName name, Strategy strategy){
		publishItem(name, strategy, null);
	}
	
	public void publishItem(ItemName name, Strategy strategy, ForwardIdentifier fID) {
		
		byte [] ridStr = name.getRendezvousId() == null? EMPTY_ARRAY : name.getRendezvousId().getId();
		byte [] scopeStr = name.getScopeId() == null? EMPTY_ARRAY : name.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.publishItem(ridStr, scopeStr, strategyValue, fidArray);
	}
	
	public void republishItem(ItemName name, ScopeID scope, Strategy strategy){
		republishItem(name, scope, strategy, null);
	}
	
	public void republishItem(ItemName name, ScopeID scope, Strategy strategy, ForwardIdentifier fid){
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");
		Util.checkNull(scope);
		
		byte [] nameStr = name.toByteArray();
		byte [] scopeStr = scope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fid == null ? null : fid.toByteArray();
		
		baWrapper.publishItem(nameStr, scopeStr, strategyValue, fidArray);
	}

	public void unpublishScope(ScopeID scope, ScopeID prefixScope,	Strategy strategy){		
		unpublishScope(scope, prefixScope, strategy, null);
	}
		
	public void unpublishScope(ScopeID scope, ScopeID prefixScope,
			Strategy strategy, ForwardIdentifier fID) {
		byte [] scopeStr = scope == null? EMPTY_ARRAY: scope.toByteArray();
		byte [] prefixStr = prefixScope == null? EMPTY_ARRAY : prefixScope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.unpublishScope(scopeStr, prefixStr, strategyValue, fidArray);		
	}
	
	public void unpublishItem(ItemName name, Strategy strategy){
		unpublishItem(name, strategy, null);
	}

	public void unpublishItem(ItemName name, Strategy strategy, ForwardIdentifier fID) {
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");
		
		
		byte [] ridStr = name.getRendezvousId() == null? EMPTY_ARRAY : name.getRendezvousId().getId();
		byte [] scopeStr = name.getScopeId() == null? EMPTY_ARRAY : name.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.unpublishItem(ridStr, scopeStr, strategyValue, fidArray);
	}

	public void subscribeScope(ScopeID scope, ScopeID prefixScope, Strategy strategy){
		subscribeScope(scope, prefixScope, strategy, null);
	}
	public void subscribeScope(ScopeID scope, ScopeID prefixScope, Strategy strategy, ForwardIdentifier fID) {		
		byte [] scopeStr = scope == null? EMPTY_ARRAY : scope.toByteArray();
		byte [] prefixStr = prefixScope == null? EMPTY_ARRAY : prefixScope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.subscribeScope(scopeStr, prefixStr, strategyValue, fidArray);
	}

	public void subscribeItem(ItemName name, Strategy strategy){
		subscribeItem(name, strategy, null);
	}
	
	public void subscribeItem(ItemName name, Strategy strategy, ForwardIdentifier fID) {
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");
		
		byte [] ridStr = name.getRendezvousId() == null? EMPTY_ARRAY : name.getRendezvousId().getId();
		byte [] scopeStr = name.getScopeId() == null? EMPTY_ARRAY : name.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
         	baWrapper.subscribeItem(ridStr, scopeStr, strategyValue, fidArray);
	}
	
	public void unsubscribeScope(ScopeID scope, ScopeID prefixScope, Strategy strategy){
		unsubscribeScope(scope, prefixScope, strategy, null);
	}

	public void unsubscribeScope(ScopeID scope, ScopeID prefixScope,
			Strategy strategy, ForwardIdentifier fID) {		
		byte [] scopeStr = scope == null? EMPTY_ARRAY : scope.toByteArray();
		byte [] prefixStr = prefixScope == null? EMPTY_ARRAY : prefixScope.toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.unsubscribeScope(scopeStr, prefixStr, strategyValue, fidArray);
	}
	
	public void unsubscribeItem(ItemName name, Strategy strategy){
		unsubscribeItem(name, strategy, null);
	}

	public void unsubscribeItem(ItemName name, Strategy strategy, ForwardIdentifier fID) {	
		Util.checkNull(name);
		Util.checkNull(name.getRendezvousId(), "no rid found in name");
		Util.checkNull(name.getScopeId(), "no scope found in name");
		
		byte [] ridStr = name.getRendezvousId() == null? EMPTY_ARRAY : name.getRendezvousId().getId();
		byte [] scopeStr = name.getScopeId() == null? EMPTY_ARRAY : name.getScopeId().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] fidArray = fID == null ? null : fID.toByteArray();
		
		baWrapper.unsubscribeItem(ridStr, scopeStr, strategyValue, fidArray);
	}
	
	public void publishData(Publication publication, Strategy strategy){
		publishData(publication, strategy, null);
	}

	public void publishData(Publication publication, Strategy strategy, ForwardIdentifier fId) {
		Util.checkNull(publication);
		Util.checkNull(publication.getItemName().getRendezvousId(), "no rid in name");
		Util.checkNull(publication.getItemName().getScopeId(), "no scope in name");
		
		byte [] str = publication.getItemName().toByteArray();
		byte strategyValue = strategy.byteValue();
		byte [] lid = fId!=null? fId.toByteArray() : null;		
		byte [] data = publication.getData();
		baWrapper.publishData(str, strategyValue, lid, data);
	}
	
	public void publishData(byte[] id, byte[] data, Strategy strategy){
		publishData(id, data, strategy, null);
	}
	public void publishData(byte[] id, byte[] data, Strategy strategy,
			ForwardIdentifier fid) {		
		byte [] lid = fid!=null? fid.toByteArray() : null;		
		baWrapper.publishData(id, strategy.byteValue(), lid, data);		
	}
	
	public void publishData(byte[] id, ByteBuffer buffer, Strategy strategy){
		publishData(id, buffer, strategy, null);
	}
	
	public void publishData(byte[] id, ByteBuffer buffer, Strategy strategy,
			ForwardIdentifier fid) {
		if(!buffer.isDirect()){			
			throw new IllegalArgumentException("Bytebuffer must be direct. Use ByteBuffer.allocateDirect() first");
		}

		byte [] lid = fid!=null? fid.toByteArray() : null;		
		baWrapper.publishData(id, strategy.byteValue(), lid, buffer);		
	}
	
	public Event getNextEvent() {
		return baWrapper.getNextEventDirect();
	}
	
	public Event getNextEvent(long timeout, TimeUnit units) throws InterruptedException{
		throw new UnsupportedOperationException("not implemented yet");
	}
	
	public void disconnect(){
		if(!closed){
			closed = true;
			baWrapper.close();
		}		
	}
	
	@Override
	protected void finalize() throws Throwable {
		disconnect();
		super.finalize();
	}		
}
