using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;

class Program
{
    static void Main(string[] args)
    {
        string mainFolderPath = args[0];
        string searchFile = args[1];
        string subZipExtension = args[2];

        string[] mainZipFiles = Directory.GetFiles(mainFolderPath, "*.zip");

        Dictionary<int, List<string>> versionsWhere = new Dictionary<int, List<string>>();

        List<string> versions = new List<string>();
        foreach (string mainZipFile in mainZipFiles)
        {
            using (ZipArchive mainArchive = ZipFile.OpenRead(mainZipFile))
            {
                foreach (ZipArchiveEntry entry in mainArchive.Entries)
                {
                    if (entry.Name.EndsWith($".{subZipExtension}"))
                    {
                        using (Stream zipStream = entry.Open())
                        using (ZipArchive subArchive = new ZipArchive(zipStream))
                        {
                            foreach (ZipArchiveEntry subEntry in subArchive.Entries)
                            {
                                if (subEntry.Name == searchFile)
                                {
                                    string version = ReadTextFileFromZipEntry(subEntry);
                                    int indexHere = 0;
                                    if ((indexHere = versions.IndexOf(version)) == -1)
                                    {
                                        indexHere = versions.Count;
                                        versions.Add(version);
                                        versionsWhere[indexHere] = new List<string>();
                                        File.WriteAllText($"{Path.GetFileNameWithoutExtension(searchFile)}_version{indexHere}.txt",version);
                                    }
                                    versionsWhere[indexHere].Add(mainZipFile);
                                }
                            }
                        }
                    }
                }
            }
        }

        StringBuilder sb = new StringBuilder();

        for(int i =0; i< versions.Count; i++)
        {

            sb.Append($"Version {i} is in the following zip files:\n");
            foreach (string zipFile in versionsWhere[i])
            {
                sb.Append($"\t{zipFile}\n");
            }
        }
        Console.WriteLine(sb.ToString());
        File.WriteAllText($"{Path.GetFileNameWithoutExtension(searchFile)}_versions.txt", sb.ToString());


        Console.ReadKey();
    }

    static string ReadTextFileFromZipEntry(ZipArchiveEntry entry)
    {
        using (StreamReader reader = new StreamReader(entry.Open()))
        {
            return reader.ReadToEnd();
        }
    }
}