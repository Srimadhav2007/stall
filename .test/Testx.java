public class Testx{
    static{
        System.loadLibrary("testx");
    }
    public native int add(int a,int b);

    public static void main(String[] args){
        Testx obj=new Testx();
        System.out.println(obj.add(2,3));
    }
}