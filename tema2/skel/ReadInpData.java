import java.io.*;
import java.util.ArrayList;
//clasa care citeste input-ul
public class ReadInpData {
    private final int dim;
    private final int nrOfFiles;
    private final ArrayList<String> files;

    //functie care citeste dimensunea, numarul de fisiere si numele fisierelor
    public ReadInpData(String inputFIle) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(inputFIle));
        this.dim = Integer.parseInt(reader.readLine());
        this.nrOfFiles = Integer.parseInt(reader.readLine());
        this.files = new ArrayList<>();
        for(int i = 0; i < nrOfFiles; i++) {
            //se construieste calea reala a fisierului
            files.add("../" + reader.readLine());
        }
    }

    public int getDim() {
        return dim;
    }

    public int getNrOfFiles() {
        return nrOfFiles;
    }

    public ArrayList<String> getFiles() {
        return files;
    }

}
