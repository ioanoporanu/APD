import java.io.*;
import java.util.*;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Tema2 {
    public static String terminalReg = "([^a-zA-Z0-9_])+";
    public static String separators = " ;:/?~\\.,><`[]{}()!@#$%^&-_+'=*\"| \t\r\n ";
    public static void main(String[] args) throws IOException, ExecutionException, InterruptedException {
        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }

        //etapa de Map
        ExecutorService tpe = Executors.newFixedThreadPool(Integer.parseInt(args[0]));
        ReadInpData input =  new ReadInpData(args[1]);
        CreateMapTasks mapTasks = new CreateMapTasks(input, tpe);
        mapTasks.createMapTasks();
        tpe.shutdown();

        //etapa de Reduce
        tpe = Executors.newFixedThreadPool(Integer.parseInt(args[0]));
        CreateReduceTasks reduceTasks = new CreateReduceTasks(mapTasks.getTaskMapList(), tpe);
        reduceTasks.createReduceTasks(input.getFiles());
        tpe.shutdown();
            for(TaskReduceResult task : reduceTasks.getTaskReduceResults()){
                System.out.println(task.getRank());
            }

        WriteOutput.writeOutput(args[2], reduceTasks.getTaskReduceResults());
    }
}
