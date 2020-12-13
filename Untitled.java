import java.util.Collections;
import java.util.*;

class Untitled {
    public static void main(String[] args) {
        ArrayList<Integer> a = new ArrayList<>();
        a.add(3);
        a.add(2);
        a.add(1);
        a.add(0);
        a.remove(Integer.valueOf(3));
        for (int i: a) {
            System.out.println(i);
        }
    }
}