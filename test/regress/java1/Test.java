import net.andrometa.neu.*;

public class Test{
	public Test(){}

	public static double static1(boolean b,
															 byte c,
															 short s,
															 int i,
															 long l,
															 float f,
															 double d){
		out("b is: " + b);
		out("c is: " + c);
		out("s is: " + s);
		out("i is: " + i);
		out("l is: " + l);
		out("f is: " + f);
		out("d is: " + d);

		return 15.5;
  }

	public nvar varRet(nvar v){
		out("v is: " + v);

		v.put("newKey", 35.4);

		return v;
  }
	
	public void varRef(nvar v){
		nvar x = new nvar();

		x.add(1);
		x.add(2);
		x.add(3);

		v.put("vec", x);
	}
	
	public String str(String str){
		return str + " modified";
	}

	static void out(String str){
		System.out.println(str);
	}
}
