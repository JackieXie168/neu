import net.andrometa.neu.*;

public class Test{
	public Test(){}

	public static void foo(int x){
		out("foo x is: " + x);
  }

	public nvar bar(nvar v){
		out("v is: " + v);

		return new nvar("baz");
  }

	static void out(String str){
		System.out.println(str);
	}
}
