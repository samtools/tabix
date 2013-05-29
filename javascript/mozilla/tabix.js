/**
 JAVASCRIPT BINDINGS for Tabix.
 
 Author: Pierre Lindenbaum PhD @yokofakun http://plindenbaum.blogspot.com
 Date: May 2013
 Reference: http://plindenbaum.blogspot.fr/2013/05/binding-c-library-with-javascript.html
 
 */

/* import js-ctypes */
Components.utils.import("resource://gre/modules/ctypes.jsm")

/** iterator constructor */
function TabixIterator(owner,ptr)
	{
	this.owner=owner;
	this.ptr=ptr;
	}

/* tabix file constructor */
function TabixFile(filename)
	{
	this.init();
	this.log("Opening "+filename);
	this.ptr=TabixFile.DLOpen(filename,0);
	this.iter=null;
	if(this.ptr.isNull()) throw "I/O ERROR: Cannot open \""+filename+"\"";
	};


/* TabixFile prototype methods below */
TabixFile.prototype.LOADED=false;
TabixFile.prototype= {
	LOADED : false,
	DEBUG : false,
	log: function(msg)
		{
		if(!this.DEBUG) return;
		print("FileReader: " + msg);
		},
	/* release the dynamic libraries */
	dispose: function()
		{
		if(!this.LOADED) return;
		this.lib.close();
		this.LOADED=false;
		},
	/* initialize the dynamic libraries */
	init : function()
		{
		if(this.LOADED) return;
		try
			{
			this.lib = ctypes.open("libtabix.so.1");
			this.log("OK loaded");
			/* https://developer.mozilla.org/en-US/docs/Mozilla/js-ctypes/js-ctypes_reference/Library#declare%28%29 */
			TabixFile.DLOpen= this.lib.declare("ti_open",
				 ctypes.default_abi,
				 ctypes.voidptr_t,
				 ctypes.char.ptr,
				 ctypes.int32_t
				 );
			TabixFile.DLClose= this.lib.declare("ti_close",
				 ctypes.default_abi,
				 ctypes.void_t,
				 ctypes.voidptr_t
				 );
			TabixFile.DLQueryS = this.lib.declare("ti_querys",
				 ctypes.default_abi,
				 ctypes.voidptr_t,
				 ctypes.voidptr_t,
				 ctypes.char.ptr
				 );
			
			TabixFile.DLQuery = this.lib.declare("ti_query",
				 ctypes.default_abi,
				 ctypes.voidptr_t,
				 ctypes.voidptr_t,
				 ctypes.int32_t,
				 ctypes.int32_t,
				 ctypes.int32_t
				 );
			
			TabixFile.DLIterFirst = this.lib.declare("ti_iter_first",
				 ctypes.default_abi,
				 ctypes.voidptr_t,
				 ctypes.voidptr_t
				 );	
			
			TabixFile.DLIterDestroy =  this.lib.declare("ti_iter_destroy",
			 	ctypes.default_abi,
				ctypes.void_t,
				ctypes.voidptr_t
				);
			
			TabixFile.DLIterRead =  this.lib.declare("ti_read",
			 	ctypes.default_abi,
				ctypes.char.ptr,
				ctypes.voidptr_t,
				ctypes.voidptr_t,
				ctypes.int32_t.ptr
				);
			this.LOADED=true;
			}
		catch(e)
			{
			this.log(e);
			throw e;
			}
		},
	/** close this TabixFile */
	close: function()
		{
		this.log("closing");
		if(this.iter!=null) this.iter.close();
		if(this.ptr==null) return;
		TabixFile.DLClose(this.ptr);
		this.ptr=null;
		},
	/** 
		returns a TABIX iterator 
	
		if argument is a string : returns an interator to the specified region : "chr:start-end"
		if argument.length==3 : invokes ti_query(ptr,arg[0],arg[1],arg[2])
		if there is no argument : return an iterator to the whole tabix file.
		
		*/
	query : function()
		{
		if(this.ptr==null) return;
		if(this.iter!=null)
			{
			this.log("Closing iterator");
			this.iter.close();
			}
		
		var r=null;
		switch(arguments.length)
			{
			case 0 : r=TabixFile.DLIterFirst(this.ptr); break;
			case 1 : r=TabixFile.DLQueryS(this.ptr,arguments[0]); break;
			case 3 : r=TabixFile.DLQuery(
				this.ptr,arguments[0],
				arguments[1],
				arguments[2]
				);break;
			default : break;
			}
		this.log("query. "+arguments.length);
		this.iter=new TabixIterator(this,r);
		return this.iter;
		}
	};

/** Iterator protorype */
TabixIterator.prototype={
	/** returns the next line or NULL */
	next: function()
		{
		if(this.ptr==null || this.owner.iter !== this) return null;
		/* https://developer.mozilla.org/en-US/docs/Mozilla/js-ctypes/Using_js-ctypes/Declaring_types */
		var len=ctypes.int32_t(0);
		var line=TabixFile.DLIterRead(this.owner.ptr,this.ptr,len.address());

		if(line.isNull())
			{
			this.close();
			return;
			}
		return line.readString();
		},
	/** release that iterator */
	close : function()
		{
		if(this.ptr!=null) TabixFile.DLIterDestroy(this.ptr);
		this.ptr=null;
		}
	}
