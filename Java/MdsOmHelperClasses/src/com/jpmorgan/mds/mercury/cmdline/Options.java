package com.jpmorgan.mds.mercury.cmdline;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;

public class Options {
	private Hashtable<String, Field> fieldVals = new Hashtable<String, Field>();
	private Hashtable<String, CmdLineOption> annotationVals = new Hashtable<String, CmdLineOption>();
	private Hashtable<String,Boolean> requiredFlags = new Hashtable<String,Boolean>();

	// Command line processing
	public static enum MdsOmCmdLineType {
		BOOL,
		BYTE,
		STRING,
		DOUBLE,
		FLOAT,
		SHORT,
		INT,
		LONG,
		CHAR,
		LIST,
		LIST_PARAM,
		HELP,
		NULL
	}
	
	private boolean getVals(){
		Field[] fields = this.getClass().getFields();
		CmdLineOption option;
		String flagName;
		
		for (Field field : fields){
			//annotation flag as the key store field
			option = field.getAnnotation(CmdLineOption.class);
			flagName = option.flag();
			//push to stack to ensure flags are fulfilled
			if(option.required())
				requiredFlags.put(flagName,false);
	
			if(validateVars(option, field)){
				if(field.getType() != null)
					fieldVals.put(flagName, field);
				annotationVals.put(flagName, option);
			}else return false;
		}
		
		return true;
	}
	
	private boolean validateVars(CmdLineOption opt, Field field){
		switch(opt.type()){
		case BOOL:
			if(!(field.getType() == boolean.class)){
				reportError(field.getName()+" does not match type boolean", opt.flag());
				return false;
			}
			break;
		case BYTE:
			if(!(field.getType() == byte.class)){
				reportError(field.getName()+" does not match type byte", opt.flag());
				return false;
			}
			break;
		case CHAR:
			if(!(field.getType() == char.class)){
				reportError(field.getName()+" does not match type char", opt.flag());
				return false;
			}
			break;
		case DOUBLE:
			if(!(field.getType() == double.class)){
				reportError(field.getName()+" does not match type double", opt.flag());
				return false;
			}
			break;
		case FLOAT:
			if(!(field.getType() == float.class)){
				reportError(field.getName()+" does not match type float", opt.flag());
				return false;
			}
			break;
		case HELP:
			break; //artificial field
		case INT:
			if(!(field.getType() == int.class)){
				reportError(field.getName()+" does not match type int", opt.flag());
				return false;
			}
			break;
		case LIST:
			try {
				if(!(field.getType() == List.class)){
					reportError(field.getName()+" does not match type list", opt.flag());
					return false;
				}else{
					field.set(null, new ArrayList<String>());
				}
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				return false;
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				return false;
			}
			break;
		case LIST_PARAM:
			try {
				if(!(field.getType() == List.class)){
					reportError(field.getName()+" does not match type list param", opt.flag());
					return false;
				}else{
					field.set(null, new ArrayList<String>());
				}
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				return false;
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				return false;
			}
			break;
		case LONG:
			if(!(field.getType() == long.class)){
				reportError(field.getName()+" does not match type long", opt.flag());
				return false;
			}
			break;
		case NULL:
			break;
		case SHORT:
			if(!(field.getType() == short.class)){
				reportError(field.getName()+" does not match type short", opt.flag());
				return false;
			}
			break;
		case STRING:
			if(!(field.getType() == String.class)){
				reportError(field.getName()+" does not match type string", opt.flag());
				return false;
			}
			break;
		default:
			break;
		}
		
		return true;
	}
	
	private boolean isFlag(String arg){
		if(arg.charAt(0) == '-' && fieldVals.containsKey(arg.substring(1, arg.length())))	
			return true;
		else
			return false;
	}
	
	/**
	 * Process argument and return specified data type
	 * @param flag
	 * @param cmdLineFlag
	 * @return
	 * @throws Exception 
	 */
	@SuppressWarnings("unchecked")
	private Object processArg(String arg, CmdLineOption cmdLineOpt) throws Exception{
		List<String> listObj;
		
		switch (cmdLineOpt.type()) {
		case BOOL:
			// match arg name and provided value
			if      (arg.equals("true")) return true;
			else if (arg.equals("false")) return false;
			else if (arg.equals("on")) return true;
			else if (arg.equals("off")) return false;
			else if (arg.equals("1")) return true;
			else if (arg.equals("0")) return false;
			else if (arg.equals("yes")) return true;
			else if (arg.equals("no")) return false;
			else if (isFlag(arg)) {				//	is not a flag
				reportError("Invalid bool entry",arg);
				throw new Exception();
			} else {
				reportError("Invalid bool argument",arg);
				throw new Exception();
			}
		case STRING:
			// match arg name and provided value
			if (!isFlag(arg)){//is not a flag
				return arg;
			}
			else {
				reportError("Invalid string argument",arg);
				throw new Exception();
			}
		case CHAR:
			// match arg name and provided value
			if (!isFlag(arg)){	//is not a flag
				return arg.charAt(0);
			}
			else {
				reportError("Invalid char argument",arg);
				throw new Exception();
			}
		case INT:
			//match arg name and provided value
			if(!isFlag(arg)){//is not a flag
				return Integer.parseInt(arg);
			}
			else {
				reportError("Invalid int argument",arg);
				throw new Exception();
			}
		case FLOAT:
			//match arg name and provided value
			if(!isFlag(arg)){//is not a flag
				return Float.parseFloat(arg);
			}
			else {
				reportError("Invalid float argument",arg);
				throw new Exception();
			}
		case DOUBLE:
			//match arg name and provided value
			if(!isFlag(arg)){//is not a flag
				return Double.parseDouble(arg);
			}
			else {
				reportError("Invalid double argument",arg);
				throw new Exception();
			}
		case SHORT:
			//match arg name and provided value
			if(!isFlag(arg)){//is not a flag
				return Short.parseShort(arg);
			}
			else {
				reportError("Invalid short argument",arg);
				throw new Exception();
			}
		case BYTE:
			//match arg name and provided value
			if(!isFlag(arg)){//is not a flag
				return Byte.parseByte(arg);
			}
			else {
				reportError("Invalid byte argument",arg);
				throw new Exception();
			}
		case LONG:
			//match arg name and provided value
			if(!isFlag(arg)){//is not a flag
				return Long.parseLong(arg);
			}
			else {
				reportError("Invalid long argument",arg);
				throw new Exception();
			}
		case HELP:
			printUsage();
			throw new Exception();
		case LIST_PARAM: 
			if(arg.length() > 0 && !isFlag(arg)){//is not a flag
				//get list
				listObj = (List<String>) fieldVals.get(cmdLineOpt.flag()).get(null);
				// Check to see if syms are comma seperated
				if(arg.indexOf(',') != -1){
					String seps = ",";
					String[] tokens = arg.split(seps);
					// Iterate over tokenized syms & add to queue
					for(int y=0; y<tokens.length; y++){
						listObj.add(new String(tokens[y]));
					}
					return listObj;
				}else{
					listObj.add(new String(arg));
					return listObj;
				}
			}
			else{
				reportError("Invalid param argument", arg);
				throw new Exception();
			}
		case LIST: 
			if(arg.length() > 0 && !isFlag(arg)){//is not a flag
				//get list object
				listObj = (List<String>) fieldVals.get(cmdLineOpt.flag()).get(null);
				listObj.add(new String(arg));
				return listObj;
			}
			break;
		case NULL:
			return arg;
		default:
			break;
		}
		//default
		return arg;
	}
	/**
	 * Returns false if any errors are encountered when processing cmd line args. Otherwise, returns true;
	 * @param args
	 * @return
	 */
	public boolean processLineArgs(String args[]){
		Field field;
		String flag;
		int errors = 0;
		Enumeration<String> flags;
		CmdLineOption cmdLineOpt;
		
		//Get fields and annotations
		if(!getVals()) //if returns false, then specified type and var type do not match
			return false;
		
		//validate only one instance MdsOmCmdLineType.LIST exists
		// - reason being that interpretation will not be deterministic
		if(!checkArgs())
			return false;
		
		//Loop over cmd line args
		for (int x = 0; x < args.length; ++x){		
			//Check to see if cmdline arg is a flag
			if(isFlag(args[x])){
				
				//strip '-' from flag name for comparison
				flag = args[x].substring(1, args[x].length());
				
				//obtain field from hashtable
				field = fieldVals.get(flag);			

				//get annotation associated with field
				cmdLineOpt = annotationVals.get(flag);
				
				try {
					//Checks to see if flag matches 
					if(flag.equals(cmdLineOpt.flag())){
						//Checks for array access will exceed argument length
						if(x+1 < args.length){
							//sets value of field
							field.set(null, processArg(args[x+1],cmdLineOpt));
							requiredFlags.put(flag, true);
						}else{ //flag with no arg
							printUsage();
							return false;
						}
						x=x+1;
					}				
				} catch (Exception e) {	//processArg throws exception if there are errors processing the values
					printUsage();
					return false;
				}
			}else{
				//obtain field from hashtable
				field = fieldVals.get(""); //list has no flag
				
				//get annotation associated with field
				cmdLineOpt = annotationVals.get("");
				
				try {
						//sets value of field
						field.set(null, processArg(args[x],cmdLineOpt));
						requiredFlags.put("", true);				
				} catch (Exception e) {	//processArg throws exception if there are errors processing the values
					printUsage();
					return false;
				}
			}
		}
		flags = requiredFlags.keys();
		
		//Check to see if all flags have been set
		while(flags.hasMoreElements()){
			flag = flags.nextElement();
		
			if(!requiredFlags.get(flag)){
				reportError("Required argument was not set",flag);
				errors++;
			}
		}
		
		//return false if all required flags were not set
		if(errors != 0){
			System.out.println();
			printUsage();
			return false;
		}
		
		return true;
	}
	
	//Validate flags will not conflict with processing
	private boolean checkArgs(){
		boolean listType=false;
		
		for(CmdLineOption val : annotationVals.values()){
			if(val.type().equals(MdsOmCmdLineType.LIST)){
				if(listType==true){
					reportError("Cannot have more than one MdsOmCmdLineType.LIST type defined",val.flag());
					return false;
				}
				else
					listType=true;
			}
		}
		
		return true;
	}
	
	//Print error for cmd line processing
	public void reportError(String desc, String argv){
		System.out.printf("Error: %s (%s)\n", desc, argv);
	}
	
	public void reportErrorUsage(String desc, String argv){
		System.out.printf("Error: %s (%s)\n", desc, argv);
		printUsage();
	}

	public void printUsage(){
		
		System.out.printf("Usage: [arg] desc [arg type]\n");
		for(CmdLineOption val : annotationVals.values()){
			String msg = "?";
			switch(val.type())
			{
				case BOOL:  		msg = "bool"; break;
				case STRING:		msg = "string"; break;
				case CHAR:			msg = "char"; break;
				case SHORT:			msg = "short"; break;
				case DOUBLE:		msg = "double"; break;
				case FLOAT:			msg = "float"; break;
				case INT:    		msg = "int"; break;
				case LONG: 			msg = "long"; break;
				case LIST:  		msg = "list"; break;
				case LIST_PARAM:	msg = "list w/ param"; break;
				case HELP:   		msg = "help"; break;
				case NULL:   		msg = "null"; break;
				default:			msg = "unknown"; break;
			}
			System.out.printf("[%s] %s (%s)\n", val.flag(), val.description(), msg);				
		}
	}		
}
