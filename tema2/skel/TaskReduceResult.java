
//clasa care cintine rezultatul de la etapa de reduce
public class TaskReduceResult {

    private final String fileName;
    private final double rank;
    private final int maxLength;
    private final int noOfWords;

    public String getFileName() {
        return fileName;
    }

    public double getRank() {
        return rank;
    }

    public int getMaxLength() {
        return maxLength;
    }

    public int getNoOfWords() {
        return noOfWords;
    }

    public TaskReduceResult(String fileName, double rank, int maxLength, int noOfWords){
        this.fileName = fileName;
        this.rank = rank;
        this.maxLength = maxLength;
        this.noOfWords = noOfWords;
    }

}
