import java.io.FileNotFoundException;
import java.util.*;
import java.util.concurrent.Callable;
import java.io.*;
import java.util.ArrayList;
import java.util.stream.Collectors;

//clasa care realizeaza etapa de Map
public class TaskMap implements Callable<TaskMapResult> {
    private final String fileName;
    private final int offset;
    private final int dim;

    public TaskMap(String fileName, int offset, int dim){
        this.fileName = fileName;
        this.offset = offset;
        this.dim = dim;
    }

    //metoda care calculeaza delay-ul pentru finalul fragmentului
    public int findLastDelay() throws IOException {
        RandomAccessFile reader = new RandomAccessFile(fileName, "r");
        reader.seek(offset + dim - 1);
        int contor = 0;
        //se numara cate caractere sunt pana la terminarea cuvantului
        int d = reader.read();
        char c =  (char) d;
        while (d != -1 && !Tema2.separators.contains(String.valueOf(c))) {
            contor++;
            d = reader.read();
            c = (char)d;
        }
        return contor;
    }

    //metoda care calculeaza delay-ul pentru inceputul fragmentului
    public int findFirstDelay() throws IOException {
        RandomAccessFile reader = new RandomAccessFile(fileName, "r");
        //daca este primul cuvant din fisier nu are rost sa caut inapoi
        if(offset <= 0){
            return 0;
        }
        reader.seek(offset - 1);

        //se numara cate caractere sunt pana la terminarea cuvantului
        int contor = 0;
        int d = reader.read();
        char c =  (char) d;
        while (d != -1 && !Tema2.separators.contains(String.valueOf(c))) {
            contor++;
            d = reader.read();
            c = (char)d;
        }
        //daca s-a ajuns la finalul fisierului inseamna ca nu exista delay pentru inceput
        if(d == -1)
            return -1;
        return contor;
    }

    @Override
    public TaskMapResult call() throws Exception {
        HashMap<Integer, Integer> mapWords = new HashMap<>();
        try {
            //se dechisde fisierul pentru citire
            RandomAccessFile reader = new RandomAccessFile(fileName, "r");
            //se calculeaza delay-urile
            int lastDelay = findLastDelay();
            int firstDelay = findFirstDelay();
            //inseamna ca acest fragment nu trebuie sa procesez niciun cuvant
            if(firstDelay == -1)
                return new TaskMapResult(new HashMap<>(), new ArrayList<>());
            //se citeste fragmentul idn fisier
            byte[] bytes = new byte[dim + lastDelay - firstDelay - 1];
            reader.seek(offset + firstDelay);
            reader.read(bytes, 0, dim + lastDelay - firstDelay - 1);
            //se formeaza vectorul de cuvinte
            List<String> words = Arrays.stream((new String(bytes)).split(Tema2.terminalReg)).filter(a -> a.length() > 0).collect(Collectors.toList());
            int maxLength = 0;

            //se creeaza dictionarul
            for(String s : words){
                if(s.length() > maxLength){
                    maxLength = s.length();
                }

                if(mapWords.containsKey(s.length())) {
                    mapWords.put(s.length(), mapWords.get(s.length()) + 1);
                } else {
                    mapWords.put(s.length(), 1);
                }
            }

            //se creeaza lista cu cuvintele de lungime maxima
            ArrayList<String> maxLengthWords = new ArrayList<>();
            for(String s : words){
                if(s.length() == maxLength){
                    maxLengthWords.add(s);
                }
            }
            //se inchide fisierul pentru citire
            reader.close();
            return  (new TaskMapResult(mapWords, maxLengthWords));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }
}
